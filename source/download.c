#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <curl/curl.h>
#include <switch.h>

#include "download.h"

#define API_AGENT           "ITotalJustice"
#define MEGABYTES_IN_BYTES	1048576

struct MemoryStruct
{
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userdata)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userdata;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (ptr == NULL) return 0;
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

int download_progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (dltotal <= 0.0) return 0;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int counter = round(tv.tv_usec / 100000);

    if (counter == 0 || counter == 2 || counter == 4 || counter == 6 || counter == 8)
    {
      printf("* DOWNLOADING: %.2fMB of %.2fMB *\r", dlnow / MEGABYTES_IN_BYTES, dltotal / MEGABYTES_IN_BYTES);
	
      struct timeval tv;
      gettimeofday(&tv, NULL);
      int counter = round(tv.tv_usec / 100000);

      if (counter == 0 || counter == 2 || counter == 4 || counter == 6 || counter == 8)
      {
          consoleUpdate(NULL);
      }
    }

	return 0;
}

int downloadFile(const char *url, const char *output, int api)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        FILE *fp = fopen(output, "wb");
        if (fp)
        {
            printf("\n");

            struct MemoryStruct chunk;
            chunk.memory = malloc(1);
            chunk.size = 0;

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, API_AGENT);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            // write calls
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

            if (api == OFF)
            {
              curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
              curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress);
            }

            // execute curl, save result
            CURLcode res = curl_easy_perform(curl);

            // write from mem to file
            fwrite(chunk.memory, 1, chunk.size, fp);

            // clean
            curl_easy_cleanup(curl);
            free(chunk.memory);
            fclose(fp);

            if (res == CURLE_OK)
            {
              printf("\n\ndownload complete!\n\n");
              consoleUpdate(NULL);
              return 0;
            }
        }
        fclose(fp);
    }
    
    printf("\n\ndownload failed...\n\n");
    consoleUpdate(NULL);
    return 1;
}
