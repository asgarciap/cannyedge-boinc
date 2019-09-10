// cannyedge_work_generator: cannyedge_app BOINC work generator.
//
// --source_dir             directory used to find images to process
// --app name               app name (default cannyedge_app)
// --in_template_file       input template file (default cannyedge_app_in)
// --out_template_file      output template file (default cannyedge_app_out)
// -d N                     log verbosity level (0..4)
// --help                   show usage
// --version                show version
//
// - Runs as a daemon, open the bmp image to process and split it in parts
//   in order to create a new job for every splited image.
// - Creates a new input file for each job;
//   the file (and the workunit names) contain a timestamp
//   and sequence number, so they're unique.

#include <sys/param.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <dirent.h>
#include <math.h>

#include "backend_lib.h"
#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "svn_version.h"
#include "util.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#include "contrib/bmp.h"

#define REPLICATION_FACTOR  1
    // number of instances of each job
#define DELAY_BOUND 900
    // max time to wait for a job to finish (900 = 15 minutes)
#define JOB_BMP_SIZE 9000000
    //estimate bmp size to generate per job (5MB)
const char* app_name = "cannyedge_app";
const char* in_template_file = "cannyedge_app_in";
const char* out_template_file = "cannyedge_app_out";
const char* source_dir = "source_dir";

char* in_template;
DB_APP app;
int start_time;
int seqno;

// create one new job
//
int make_job(const char* name) {
    DB_WORKUNIT wu;
    char path[MAXPATHLEN];
    const char* infiles[1];
    int retval;

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    retval = config.download_path(name, path);
    if (retval) return retval;
    rename(name,path);

    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    safe_strcpy(wu.name, name);
    wu.rsc_fpops_est = 1e12;
    wu.rsc_fpops_bound = 1e14;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = DELAY_BOUND;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;
    wu.app_version_num = 200;
    infiles[0] = name;

    // Register the job with BOINC
    //
    sprintf(path, "templates/%s", out_template_file);
    return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles,
        1,
        config
    );
}

void main_loop() {
    int retval;
    DIR* dir;
    while(1) {
        check_stop_daemons();
        long n;
        retval = count_unsent_results(n, app.id);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "count_unsent_jobs() failed: %s\n", boincerror(retval)
            );
            exit(retval);
        }
        if (n) {
            log_messages.printf(MSG_NORMAL,"waiting for current file to finish.\n");
            daemon_sleep(10);
        }else {
            struct dirent *ent;
            if((dir = opendir(source_dir)) != NULL) {
                while((ent = readdir(dir)) != NULL) {
                if(!ent || (strcmp(ent->d_name,".") == 0) || (strcmp(ent->d_name,"..") == 0) ) {
                    log_messages.printf(MSG_NORMAL,"no files found to process.\n");
                    daemon_sleep(10);
                }else {
                    log_messages.printf(MSG_NORMAL, "processing file: %s/%s\n",source_dir,ent->d_name);
                    char bmpfile[1024];
                    snprintf(bmpfile,sizeof(bmpfile),"%s/%s",source_dir,ent->d_name);
                    Bitmap* bm = bm_load(bmpfile);
                    if(bm) {
                        log_messages.printf(MSG_NORMAL, "bmp size. h: %d w: %d\n",bm->h,bm->w);
                        long fsize = bm->h*bm->w*4;
                        if(bm->w > bm->h) {
                            //split vertically
                            char filepartname[1024];
                            int wsize = JOB_BMP_SIZE;
                            if(fsize > JOB_BMP_SIZE) 
                                wsize = ceil(bm->w/ceil(fsize/JOB_BMP_SIZE));
                            for(int i=0;i<bm->w;) {
                                Bitmap* bmpart = bm_crop(bm,i,0,wsize,bm->h);
                                snprintf(filepartname,sizeof(filepartname),"%s_%d_%d_%s",app_name,start_time,seqno++,ent->d_name);
                                bm_save(bmpart,filepartname);
                                log_messages.printf(MSG_NORMAL, "creating work for file: %s - h: %d w:%d (%d,0)\n",filepartname,bmpart->h,bmpart->w,i);
                                make_job(filepartname);
                                i+=bmpart->w;                            
                                bm_free(bmpart);
                            }
                        }else {
                            //split horizontally
                            char filepartname[1024];
                            int hsize = JOB_BMP_SIZE;
                            if(fsize > JOB_BMP_SIZE)
                                hsize = ceil(bm->h/ceil(fsize/JOB_BMP_SIZE));
                            for(int i=0;i<bm->h;) {
                                Bitmap* bmpart = bm_crop(bm,0,i,bm->w,hsize);
                                snprintf(filepartname,sizeof(filepartname),"%s_%d_%d_%s",app_name,start_time,seqno++,ent->d_name);
                                bm_save(bmpart,filepartname);
                                log_messages.printf(MSG_NORMAL, "creating work for file: %s - h: %d w: %d\n",filepartname,bmpart->h,bmpart->w);
                                make_job(filepartname);
                                i+=bmpart->h;
                                bm_free(bmpart);
                            }
                        }
                        bm_free(bm);
                    }else {
                        log_messages.printf(MSG_CRITICAL,"could not open file: %s as a bmp image. %s\n",bmpfile,bm_last_error);
                        daemon_sleep(5);
                    } 
                    unlink(bmpfile);
                }
                } //elihw
                closedir(dir);
            }else {
                log_messages.printf(MSG_CRITICAL,
                        "could not open source_dir: %s\n", source_dir
                    );
                    exit(1);
            }
        }
    }
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ --app X                Application name (default: example_app)\n"
        "  [ --in_template_file     Input template (default: example_app_in)\n"
        "  [ --out_template_file    Output template (default: example_app_out)\n"
        "  [ -d X ]                 Sets debug level to X.\n"
        "  [ -h | --help ]          Shows this help text.\n"
        "  [ -v | --version ]       Shows version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "--source_dir")) {
            source_dir = argv[++i];            
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--in_template_file")) {
            in_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--out_template_file")) {
            out_template_file = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    snprintf(buf, sizeof(buf), "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    snprintf(buf, sizeof(buf), "templates/%s", in_template_file);
    if (read_file_malloc(config.project_path(buf), in_template)) {
        log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
        exit(1);
    }

    start_time = time(0);
    seqno = 0;

    log_messages.printf(MSG_NORMAL, "Starting\n");

    main_loop();
}
