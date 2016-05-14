#include <vector>
#include <atomic>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>

#include "SafeQueue.h"
namespace MobileNoise
{
	namespace SOS
	{
		class CachingStream
		{
		public:

			static const size_t maxFilePathSize = 4096;
			static const size_t nextCharsToErasePrevPiece = 4096;
			static const size_t nextFilesToErasePrevPiece = 3;

			CachingStream();
			~CachingStream();

			void insertString(const std::string & inputStr);
			bool get(char & ch);
			size_t getBeg();
			size_t tellg();
			size_t getEnd();
			std::stringstream getStrStream();
			bool isStreaming();
			void beginStreaming();
			void endStreaming();
			bool hasData();
			void terminate();


		private:

			SafeQueue<std::string> inputQueue;
			std::mutex cacheFilesMut;
			std::vector<std::string> cacheFiles;
			std::mutex backedFilesMut;
			std::vector<std::string> backedFiles;
			std::string currentFile;
			std::atomic<bool> streaming;
			std::atomic<bool> good;
			std::atomic<bool> m_terminate;
			const std::string pathToCachingFiles;

			//std::thread * cachingThread;
			std::thread * cachingThread;
			std::atomic<bool> isCaching;

			std::mutex sprintfMut;

			void cache();
			std::atomic<size_t> fileNo;

			size_t cacheFileNameSize;
			std::atomic<size_t> charPos;
			std::atomic<size_t> pastFiles;

			std::ifstream openedFile;

			void delPastFile();
			void delPastFileTh();
	  


		};
	} // namespace SOS
} // namespace MobileNoise