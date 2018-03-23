#include "ps4.h"
#include "pkg.h"
#include "patch.h"


#define INI_FILE "AppToUsb.ini"

int nthread_run;
char notify_buf[1024];
char ini_file_path[255];
int  xfer_pct;
long xfer_cnt;
char *cfile;
int tmpcnt;
int isxfer;


void systemMessage(char* msg) {
 	char buffer[512]; 
 	sprintf(buffer, "%s\n\n\n\n\n\n\n", msg);
 	sceSysUtilSendSystemNotificationWithText(0x81, buffer);
}

int file_compare(char *fname1, char *fname2)
{
    long size1, size2;

    int  bytesRead1 = 0,
         bytesRead2 = 0,
         lastBytes = 100,
         res = 0,
         i;

    FILE *file1 = fopen(fname1, "rb"),
         *file2 = fopen(fname2, "rb");

    char *buffer1 = malloc(65536),
         *buffer2 = malloc(65536);

    if (!file1 || !file2) {
        return res;
    }

    fseek (file1, 0, SEEK_END);
    fseek (file2, 0, SEEK_END);

    size1 = ftell (file1);
    size2 = ftell (file2);

    fseek(file1, 0L, SEEK_SET);
    fseek(file2, 0L, SEEK_SET);

    if (size1 != size2) {
        res = 0;
        goto exit;
    }


    if (size1 < lastBytes) lastBytes = size1;

    fseek(file1, -lastBytes, SEEK_END);
    fseek(file2, -lastBytes, SEEK_END);

    bytesRead1 = fread(buffer1, sizeof(char), lastBytes, file1);
    bytesRead2 = fread(buffer2, sizeof(char), lastBytes, file2);

    if (bytesRead1 > 0 && bytesRead1 == bytesRead2) {
        for ( i = 0; i < bytesRead1; i++) {
            if (buffer1[i] != buffer2[i]) {
                res = 0;
                goto exit;
            }
        }
        res = 1;
    }

    free(buffer1);
    free(buffer2);

    exit:
    fclose(file1);
    fclose(file2);
    return res;
}



char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig))) 
    return str;

  strncpy(buffer, str, p-str);
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}


int fgetc(FILE *fp)
{
  char c;

  if (fread(&c, 1, 1, fp) == 0)
    return (-1);
  return (c);
}


char *read_string(FILE* f)
{
    char *string = malloc(sizeof(char) * 65535);
    int c;
    int length = 0;
    if (!string) return string;
    while((c = fgetc(f)) != -1)
    {
        string[length++] = c;
    }
    string[length++] = '\0';

    return realloc(string, sizeof(char) * length);
}


int file_exists(char *fname)
{
    FILE *file = fopen(fname, "rb");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int dir_exists(char *dname)
{
    DIR *dir = opendir(dname);

    if (dir)
    {
        closedir(dir);
        return 1;
    }
    return 0;
}


int symlink_exists(const char* fname)
{
    struct stat statbuf;
    if (lstat(fname, &statbuf) < 0) {
        return -1;
    }
    if (S_ISLNK(statbuf.st_mode) == 1) {
        return 1;
    } else {
        return 0;
    }
}


void makeini()
{
    if (!file_exists(ini_file_path)) 
    {
    FILE *ini = fopen(ini_file_path,"wb");
    char *buffer;
    buffer ="To check the usb root for the pkg file to save time copying from the internal ps4 drive then uncomment the line below.\r\nbut remember this will move the pkg from the root directory to the PS4 folder.\r\n//CHECK_USB\r\n\r\nTo leave game updates on the internal drive uncomment the line below\r\n//IGNORE_UPDATES\r\n\r\nTo use this list as a list of games you want to move not ignore then uncomment the line below.\r\n//MODE_MOVE\r\n\r\nExample ignore or move usage.\r\n\r\nCUSAXXXX1\r\nCUSAXXXX2\r\nCUSAXXXX3";
    fwrite(buffer, 1, strlen(buffer), ini);
    fclose(ini);
    }
}

int isinlist(char *sourcefile)
{
        if (file_exists(ini_file_path)) 
        {
            FILE *cfile = fopen(ini_file_path, "rb");
            char *idata = read_string(cfile);
            fclose(cfile);
            if (strlen(idata) != 0)
            {
             char *tmpstr;
                if (strstr(sourcefile, "/user/app/") != NULL)
                {
                tmpstr = replace_str(sourcefile, "/user/app/", "");
                tmpstr = replace_str(tmpstr, "/app.pkg", "");
                }
                else
                {
                tmpstr = replace_str(sourcefile, "/user/patch/", "");
                tmpstr = replace_str(tmpstr, "/patch.pkg", "");
                }
                if(strstr(idata, tmpstr) != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


int ismovemode()
{
        if (file_exists(ini_file_path)) 
        {
            FILE *cfile = fopen(ini_file_path, "rb");
            char *idata = read_string(cfile);
            fclose(cfile);
            if (strlen(idata) != 0)
            {
                if(strstr(idata, "//MODE_MOVE") != NULL) 
                {
                   return 0;
                }
                else if(strstr(idata, "MODE_MOVE") != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


int isusbcheck()
{
        if (file_exists(ini_file_path)) 
        {
            FILE *cfile = fopen(ini_file_path, "rb");
            char *idata = read_string(cfile);
            fclose(cfile);
            if (strlen(idata) != 0)
            {
                if(strstr(idata, "//CHECK_USB") != NULL) 
                {
                   return 0;
                }
                else if(strstr(idata, "CHECK_USB") != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


int isignupdates()
{
        if (file_exists(ini_file_path)) 
        {
            FILE *cfile = fopen(ini_file_path, "rb");
            char *idata = read_string(cfile);
            fclose(cfile);
            if (strlen(idata) != 0)
            {
                if(strstr(idata, "//IGNORE_UPDATES") != NULL) 
                {
                   return 0;
                }
                else if(strstr(idata, "IGNORE_UPDATES") != NULL) 
                {
                   return 1;
                }
             return 0;
             }
        return 0;
        }
        else
        {
             return 0;
        }
}


void copyFile(char *sourcefile, char* destfile)
{
    FILE *src = fopen(sourcefile, "rb");
    if (src)
    {
        FILE *out = fopen(destfile,"wb");
        if (out)
        {
            cfile = sourcefile;
            isxfer = 1;
            size_t bytes, bytes_size, bytes_copied = 0;
            char *buffer = malloc(65536);
            if (buffer != NULL)
            {
                fseek(src, 0L, SEEK_END);
                bytes_size = ftell(src);
                fseek(src, 0L, SEEK_SET);
                while (0 < (bytes = fread(buffer, 1, 65536, src))) {
                    fwrite(buffer, 1, bytes, out);
                    bytes_copied += bytes;
                    if (bytes_copied > bytes_size) bytes_copied = bytes_size;
                   xfer_pct = bytes_copied * 100 / bytes_size;
                   xfer_cnt += bytes;
                }
                free(buffer);
            }
            fclose(out);
            isxfer = 0;
            xfer_pct = 0;
            xfer_cnt = 0;
            unlink(sourcefile);
            symlink(destfile, sourcefile);
        }
        else {
        }
        fclose(src);
    }
    else {
    }
}



void copypkg(char *sourcepath, char* destpath)
{       
  char cmsg[1024];
	if (!symlink_exists(sourcepath))
        {
        if (isfpkg(sourcepath) == 0) 
        {
            if (!file_exists(destpath)) 
            {
                sprintf(cmsg, "%s\n%s", "Processing:" , sourcepath);
                systemMessage(cmsg);
                copyFile(sourcepath, destpath);
            }
            else
            {
                if (!file_compare(sourcepath, destpath))
                {
                    sprintf(cmsg, "%s\n%s\nOverwriting as pkg files are mismatched", "Found pkg at " , destpath);
                    systemMessage(cmsg);
                    copyFile(sourcepath, destpath);
                } 
                else
                {  
                    sprintf(cmsg, "%s\n%s\nSkipping copy and linking existing pkg", "Found pkg at " , destpath);
                    systemMessage(cmsg);
                    sceKernelSleep(5);
                    unlink(sourcepath);
                    symlink(destpath, sourcepath);
                }
            }
        }
    }
}


void checkusbpkg(char *sourcedir, char* destdir) {
   if (isusbcheck())
   {
     if (!symlink_exists(sourcedir))
     {
        if (isfpkg(sourcedir) == 0) 
        {
            if (!file_exists(destdir)) 
            {
            DIR *dir;
            struct dirent *dp;
            struct stat info;
            char upkg_path[1024];
            dir = opendir("/mnt/usb0/");
            if (dir) {
            while ((dp = readdir(dir)) != NULL)
            {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
            {}
            else
            {
            sprintf(upkg_path, "%s/%s", "/mnt/usb0", dp->d_name);
            if (!stat(upkg_path, &info))
            {
                if (S_ISREG(info.st_mode))
                {
		if (file_compare(sourcedir, upkg_path))
		{
		    rename(upkg_path, destdir);
		    break;
	        }
             }
          }
        }
      }
      closedir(dir);
      }
     }
   }
 }
}
}



void copyDir(char *sourcedir, char* destdir)
{
    DIR *dir;
    struct dirent *dp;
    struct stat info;
    char src_path[1024], dst_path[1024];
    dir = opendir(sourcedir);
    if (!dir)
        return;
        mkdir(destdir, 0777);
    while ((dp = readdir(dir)) != NULL)
    {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        {}
        else
        {
            sprintf(src_path, "%s/%s", sourcedir, dp->d_name);
            sprintf(dst_path, "%s/%s", destdir  , dp->d_name);

            if (!stat(src_path, &info))
            {
                if (S_ISDIR(info.st_mode))
                {
                  copyDir(src_path, dst_path);
                }
                else
                if (S_ISREG(info.st_mode))
                {
                  if(strstr(src_path, "app.pkg") != NULL || strstr(src_path, "patch.pkg") != NULL) 
                  {
                   if (ismovemode() )
                   {
                   if (isinlist(src_path) )
                   {
                     checkusbpkg(src_path, dst_path);
                     copypkg(src_path, dst_path);
                   }
                 }
                 else
                 {
                   if (!isinlist(src_path) )
                   {
                     checkusbpkg(src_path, dst_path);
                     copypkg(src_path, dst_path);
                   }
                 }
               }
             }
          }
       }
    }
    closedir(dir);
}



void *nthread_func(void *arg)
{
        time_t t1, t2;
        t1 = 0;
	while (nthread_run)
	{
		if (isxfer)
		{
			t2 = time(NULL);
			if ((t2 - t1) >= 20)
			{
				t1 = t2;
				if (tmpcnt >= 1048576)
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u MB/s", cfile , xfer_pct, tmpcnt / 1048576);
				}
				else if (tmpcnt >= 1024)
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u KB/s", cfile , xfer_pct, tmpcnt / 1024);
				}
				else
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u B/s", cfile , xfer_pct, tmpcnt);
				}
				
				systemMessage(notify_buf);
			}
		}
		else t1 = 0;
		sceKernelSleep(1);
	}
	return NULL;
}



void *sthread_func(void *arg)
{
	while (nthread_run)
	{
           if (isxfer)
           {
              tmpcnt = xfer_cnt;
              xfer_cnt = 0;
           }
          sceKernelSleep(1);
	}
	return NULL;
}



int _main(struct thread *td) {
    initKernel();
    initLibc();
    initPthread();
    DIR *dir;
    dir = opendir("/user/app");
    if (!dir)
    {
       syscall(11,patcher,td);
    }
    else
    {
       closedir(dir);
    }
    initSysUtil();
        xfer_cnt = 0;
        isxfer = 0;
	nthread_run = 1;
	ScePthread nthread;
	scePthreadCreate(&nthread, NULL, nthread_func, NULL, "nthread");
	ScePthread sthread;
	scePthreadCreate(&sthread, NULL, sthread_func, NULL, "sthread");
        systemMessage("Warning this payload will modify the filesystem on your PS4\n\nUnplug your usb drive to cancel this");
        sceKernelSleep(10);
        systemMessage("Last warning\n\nUnplug your usb drive to cancel this");
        sceKernelSleep(10);

           FILE *usbdir = fopen("/mnt/usb0/.dirtest", "wb");
         if (!usbdir)
            {
                  systemMessage("No usb mount found.\nYou must use a eXfat formatted usb hdd\nThe USB drive must be plugged into USB0");
                  nthread_run = 0;
                  return 0;
            }
            else
            {
                        fclose(usbdir);
                        unlink("/mnt/usb0/.dirtest");
                        mkdir("/mnt/usb0/PS4/", 0777);
                        sprintf(ini_file_path, "/mnt/usb0/%s", INI_FILE);
                        if (!file_exists(ini_file_path))
                        {
                        sprintf(ini_file_path, "/mnt/usb0/PS4/%s", INI_FILE);
                        makeini();
                        }
                        systemMessage("Copying Apps to USB0\n\nThis will take a while if you have lots of games installed");
                        copyDir("/user/app","/mnt/usb0/PS4");
                        if (!isignupdates())
                        {
                           mkdir("/mnt/usb0/PS4/updates/", 0777);
                           systemMessage("Copying updates to USB0");
                           copyDir("/user/patch","/mnt/usb0/PS4/updates");
                        }
                        systemMessage("Complete.");
            }
    nthread_run = 0;
    return 0;
}



