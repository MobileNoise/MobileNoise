#include <iostream>
#include <sstream>
#include <string>
#include <curl.h>
#include <atomic>
#include "CachingStream.h"
//
//int main(void)
//{
//  CURL *curl;
//  CURLcode res;
//  std::string content;
//
//  curl = curl_easy_init();
//  if(curl) {
//    curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com");
//    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
//    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
//    res = curl_easy_perform(curl);
//    curl_easy_cleanup(curl);
//
//    std::ostringstream out;
//    out << res;
//
//    content = out.str();
//
//	std::cout << content << std::endl;
//  }
//  return 0;
//}

//#include <iostream>
//#include <string>
//#include <curl.h>
//
//
//static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
//{
//    ((std::string*)userp)->append((char*)contents, size * nmemb);
//    return size * nmemb;
//}
//
//int main(void)
//{
//  CURL *curl;
//  CURLcode res;
//  std::string readBuffer;
//
//  curl = curl_easy_init();
//  if(curl) {
//    curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com");
//    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
//    res = curl_easy_perform(curl);
//    curl_easy_cleanup(curl);
//
//    std::cout << readBuffer << std::endl;
//  }
//  return 0;
//}

//#define USE_CHUNKED
//
//const char data[]="{ \"request\": \"GetObservation\",\"service\": \"SOS\",\"version\": \"2.0.0\",\"procedure\": \"urn.meters.sound.mobile.phones.MACNotFound\",\"offering\": \"http://mobile-noise.ddns.net/offering/urn:muni:def:network:mobile:noise2015/observations\",\"observedProperty\": \"http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise\",\"featureOfInterest\": \"urn:muni:def:world:environment:noise\"}";
// 
//struct WriteThis {
//  const char *readptr;
//  long sizeleft;
//};
// 
//static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
//{
//  struct WriteThis *pooh = (struct WriteThis *)userp;
// 
//  if(size*nmemb < 1)
//    return 0;
// 
//  if(pooh->sizeleft) {
//    *(char *)ptr = pooh->readptr[0]; /* copy one single byte */ 
//    pooh->readptr++;                 /* advance pointer */ 
//    pooh->sizeleft--;                /* less data left */ 
//    return 1;                        /* we return 1 byte at a time! */ 
//  }
// 
//  return 0;                          /* no more data left to deliver */ 
//}
// 
//int main(void)
//{
//  CURL *curl;
//  CURLcode res;
// 
//  struct WriteThis pooh;
// 
//  pooh.readptr = data;
//  pooh.sizeleft = (long)strlen(data);
// 
//  /* In windows, this will init the winsock stuff */ 
//  res = curl_global_init(CURL_GLOBAL_DEFAULT);
//  /* Check for errors */ 
//  if(res != CURLE_OK) {
//    fprintf(stderr, "curl_global_init() failed: %s\n",
//            curl_easy_strerror(res));
//    return 1;
//  }
// 
//  /* get a curl handle */ 
//  curl = curl_easy_init();
//  if(curl) {
//    /* First set the URL that is about to receive our POST. */ 
//    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/52n-sos-webapp/service");
// 
//    /* Now specify we want to POST data */ 
//    curl_easy_setopt(curl, CURLOPT_POST, 1L);
// 
//    /* we want to use our own read function */ 
//    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
// 
//    /* pointer to pass to our read function */ 
//    curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);
// 
//    /* get verbose debug output please */ 
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
// 
//    /*
//      If you use POST to a HTTP 1.1 server, you can send data without knowing
//      the size before starting the POST if you use chunked encoding. You
//      enable this by adding a header like "Transfer-Encoding: chunked" with
//      CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must
//      specify the size in the request.
//    */ 
//#ifdef USE_CHUNKED
//    {
//      struct curl_slist *chunk = NULL;
// 
//      chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
//      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
//      /* use curl_slist_free_all() after the *perform() call to free this
//         list again */ 
//    }
//#else
//    /* Set the expected POST size. If you want to POST large amounts of data,
//       consider CURLOPT_POSTFIELDSIZE_LARGE */ 
//    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pooh.sizeleft);
//#endif
// 
//#ifdef DISABLE_EXPECT
//    /*
//      Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
//      header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
//      NOTE: if you want chunked transfer too, you need to combine these two
//      since you can only set one list of headers with CURLOPT_HTTPHEADER. */ 
// 
//    /* A less good option would be to enforce HTTP 1.0, but that might also
//       have other implications. */ 
//    {
//      struct curl_slist *chunk = NULL;
// 
//      chunk = curl_slist_append(chunk, "Expect:");
//      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
//      /* use curl_slist_free_all() after the *perform() call to free this
//         list again */ 
//    }
//#endif
// 
//    /* Perform the request, res will get the return code */ 
//    res = curl_easy_perform(curl);
//    /* Check for errors */ 
//    if(res != CURLE_OK)
//      fprintf(stderr, "curl_easy_perform() failed: %s\n",
//              curl_easy_strerror(res));
// 
//    /* always cleanup */ 
//    curl_easy_cleanup(curl);
//
//	std::ostringstream out;
//    out << res;
//
//    //content = out.str();
//
//	std::cout << out << std::endl;
//
//  }
//  curl_global_cleanup();
//  return 0;
//}

//static std::string *DownloadedResponse;
//static std::string URL = "http://localhost:8080/52n-sos-webapp/service";
//
//static int writer(char *data, size_t size, size_t nmemb, std::string *buffer_in)
//{
//
//    // Is there anything in the buffer?  
//    if (buffer_in != NULL)  
//    {
//        // Append the data to the buffer    
//        buffer_in->append(data, size * nmemb);
//
//        // How much did we write?   
//        DownloadedResponse = buffer_in;
//
//        return size * nmemb;  
//    }
//
//    return 0;
//
//} 
//
////std::string ServerContent::DownloadJSON(std::string URL)
//int main(void)
//{   
//    CURL *curl;
//    CURLcode res;
//    struct curl_slist *headers=NULL; // init to NULL is important 
//    std::ostringstream oss;
//    curl_slist_append( headers, "Accept: application/json");  
//    curl_slist_append( headers, "Content-Type: application/json");
//    curl_slist_append( headers, "charsets: utf-8"); 
//    curl = curl_easy_init();
//
//    if (curl) 
//    {
//        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
//        curl_easy_setopt(curl, CURLOPT_HTTPGET,1); 
//        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writer);
//        res = curl_easy_perform(curl);
//
//        if (CURLE_OK == res) 
//        { 
//            char *ct;         
//            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
//            if((CURLE_OK == res) && ct)
//			{
//				std::string res(*DownloadedResponse);
//				curl_slist_free_all(headers);
//                return 0;
//			}
//        }
//    }
//	curl_slist_free_all(headers);
//	return 1;
//}  

#define USE_CHUNKED
std::stringstream outputStream;
static std::string *DownloadedResponse;
//SafeQueue<std::stringstream> inputStringStrQueue;
static int not = 0;
std::atomic<bool> isConnected = false;
MobileNoise::SOS::CachingStream cs;

const char data[]="{ \"request\": \"GetObservation\",\"service\": \"SOS\",\"version\": \"2.0.0\",\"procedure\": \"urn.meters.sound.mobile.phones.MACNotFound\",\"offering\": \"http://mobile-noise.ddns.net/offering/urn:muni:def:network:mobile:noise2015/observations\",\"observedProperty\": \"http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise\",\"featureOfInterest\": \"urn:muni:def:world:environment:noise\"}";
 
struct WriteThis {
  const char *readptr;
  long sizeleft;
};

void notified(void)
{
	not++;
}
 
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *pooh = (struct WriteThis *)userp;
 
  if(size*nmemb < 1)
    return 0;
 
  if(pooh->sizeleft) {
    *(char *)ptr = pooh->readptr[0]; /* copy one single byte */ 
    pooh->readptr++;                 /* advance pointer */ 
    pooh->sizeleft--;                /* less data left */ 
    return 1;                        /* we return 1 byte at a time! */ 
  }
 
  return 0;                          /* no more data left to deliver */ 
}

static std::string URL = "http://localhost:8080/52n-sos-webapp/service";

static int writer(char *data, size_t size, size_t nmemb, std::string *buffer_in)
{
	std::stringstream inputStream;//(data, size * nmemb);

	for (size_t i = 0; i < (size * nmemb); i++)
	{
		inputStream << *data;
		data++;
	}

	cs.insertString(inputStream.str());

//	inputStringStrQueue.enqueue(inputStream);

	return size * nmemb;  

  //  // Is there anything in the buffer?  
  //  if (buffer_in != NULL)  
  //  {
  //      // Append the data to the buffer    
  //      buffer_in->append(data, size * nmemb);

		//std::string bufferOut = *buffer_in;

  //      // How much did we write?   
  //      //outputStream << bufferOut;

  //      return size * nmemb;  
  //  }

  //  return 0;

} 
 
int main1(void)
{

  CURL *curl;
  CURLcode res;
 
  struct WriteThis pooh;
 
  pooh.readptr = data;
  pooh.sizeleft = (long)strlen(data);
 
  /* In windows, this will init the winsock stuff */ 
  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  /* Check for errors */ 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n",
            curl_easy_strerror(res));
    return 1;
  }

	struct curl_slist *headers=NULL; // init to NULL is important 
	std::ostringstream oss;
	headers = curl_slist_append( headers, "Accept: application/json");  
	headers = curl_slist_append( headers, "Content-Type: application/json");
	headers = curl_slist_append( headers, "charsets: utf-8"); 
 
  /* get a curl handle */ 
  curl = curl_easy_init();
  if(curl) {

	//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    /* First set the URL that is about to receive our POST. */ 
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/52n-sos-webapp/service");
 
    /* Now specify we want to POST data */ 
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
 
    /* we want to use our own read function */ 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writer);
 
    /* pointer to pass to our read function */ 
    curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);


 
    /* get verbose debug output please */ 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
    /*
      If you use POST to a HTTP 1.1 server, you can send data without knowing
      the size before starting the POST if you use chunked encoding. You
      enable this by adding a header like "Transfer-Encoding: chunked" with
      CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must
      specify the size in the request.
    */ 
#ifdef USE_CHUNKED
    {
      //struct curl_slist *chunk = NULL;
 
      headers = curl_slist_append(headers, "Transfer-Encoding: chunked");
      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      /* use curl_slist_free_all() after the *perform() call to free this
         list again */ 
    }
#else
    /* Set the expected POST size. If you want to POST large amounts of data,
       consider CURLOPT_POSTFIELDSIZE_LARGE */ 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pooh.sizeleft);
#endif
 
#ifdef DISABLE_EXPECT
    /*
      Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
      header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
      NOTE: if you want chunked transfer too, you need to combine these two
      since you can only set one list of headers with CURLOPT_HTTPHEADER. */ 
 
    /* A less good option would be to enforce HTTP 1.0, but that might also
       have other implications. */ 
    {
      struct curl_slist *chunk = NULL;
 
      chunk = curl_slist_append(chunk, "Expect:");
      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
      /* use curl_slist_free_all() after the *perform() call to free this
         list again */ 
    }
#endif

	isConnected = true;
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);

	cs.endStreaming();
	isConnected = false;

    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);

	std::ostringstream out;
    out << res;

    //content = out.str();

	std::cout << out.str() << std::endl;
	//std::cout << outputStream.str() << std::endl;

	std::cout << cs.getStrStream().str().c_str() << std::endl;



	if (CURLE_OK == res) 
    { 
        char *ct;         
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
        if((CURLE_OK == res) && ct)
		{
			//while (!inputStringStrQueue.empty())
			{
				//std::cout << inputStringStrQueue.dequeue().str();
			}


			//std::cout << outputStream << std::endl;
			//std::string res(*DownloadedResponse);
		}
    }

	//std::cout << *DownloadedResponse << std::endl;

  }
  curl_slist_free_all(headers);
  curl_global_cleanup();
  return 0;
}