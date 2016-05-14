#define _CRT_SECURE_NO_WARNINGS

#include "CachingStream.h"
#include <exception>


namespace MobileNoise
{
	namespace SOS
	{
		CachingStream::CachingStream()
			: cachingThread(nullptr)
			, charPos(0)
			, pastFiles(0)
			, good(false)
		{
			fileNo = 0;
			streaming = false;
		}

		CachingStream::~CachingStream()
		{
			terminate();
		}

		void CachingStream::insertString(const std::string & inputStr)
		{
			beginStreaming();
			inputQueue.enqueue(inputStr);
		}

		void CachingStream::beginStreaming()
		{
			m_terminate = false;
			streaming = true;
			if (!cachingThread)
			{
				cachingThread = new std::thread(&MobileNoise::SOS::CachingStream::cache, this);
			}
		}

		void CachingStream::endStreaming()
		{
			streaming = false;

			if (cachingThread)
			{
				if (cachingThread->joinable())
				{
					cachingThread->join();
				}
				if (cachingThread)
				{
					delete cachingThread;
				}
				cachingThread = nullptr;
			}

		}

		bool CachingStream::isStreaming()
		{
			return streaming;
		}

		std::stringstream CachingStream::getStrStream()
		{
			std::stringstream output;
			char ch;

			while (get(ch))
			{
				output << ch;
			} 
			return output;
		}

		bool CachingStream::get(char & ch)
		{
			if (openedFile)
			{
				ch = openedFile.get();
				if (!openedFile.eof())
				{
					good = true;
					if (charPos++ > nextCharsToErasePrevPiece)
					{
						delPastFile();
						charPos = 1;
					}
					return good;
				}
				else if (!currentFile.empty())
				{
					// close old file, open next file, continue streaming
					openedFile.close();
					std::unique_lock<std::mutex>lock(backedFilesMut);
					backedFiles.push_back(currentFile);
					pastFiles++;
				}
			}

			// any file is not open
			if (!openedFile.is_open())
			{

				// all files empty
				while (true)
				{
					bool fileopen = false;
					{ // scope for lock
						std::unique_lock<std::mutex>lock(cacheFilesMut);
						if (!cacheFiles.empty())
						{
							currentFile = cacheFiles[0];
							openedFile.open(currentFile);
							cacheFiles.erase(cacheFiles.begin());
							fileopen = true;
						}	
					}

					if (fileopen)
					{
						return get(ch);
					}

					if (streaming)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
					}
					else
					{
						good = false;
						ch = -1;
						return good;
					}
				}
			}

			std::cout << "CachingStream::get: ERROR when reading file " << currentFile.c_str() << ", pos: " 
				<< openedFile.showpos << ", badbit: " << openedFile.badbit << ", failbit: " << openedFile.failbit
				<< ", eof: " << openedFile.eofbit << std::endl;
			good = false;
			ch = -1;
			return good;
		}

		bool CachingStream::hasData()
		{
			return isCaching || isStreaming() || (!inputQueue.empty() || (openedFile && openedFile.good()));
		}

		void CachingStream::terminate()
		{
			m_terminate = true;
			endStreaming();

			if (!currentFile.empty())
			{
				openedFile.close();
				std::unique_lock<std::mutex>lock(backedFilesMut);
				backedFiles.push_back(currentFile);
				currentFile = "";
			}

			while (true)
			{
				{ // lock scope
					std::unique_lock<std::mutex>lock(backedFilesMut);
					if (backedFiles.empty())
					{
						break;
					}
				}
				delPastFileTh();
			}
		}

		void CachingStream::cache()
		{
			isCaching = true;
			while ((isStreaming() || (!inputQueue.empty())) && !m_terminate)
			{
				if (inputQueue.empty())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}
				
				char fileName [maxFilePathSize];
				{
					std::unique_lock<std::mutex>slock(sprintfMut);
					if(maxFilePathSize < (pathToCachingFiles.size() + sprintf(fileName,"%d",fileNo) + 11))
					{
						std::cout << "cache: Path and filename character string is too long, max allowed size is " << maxFilePathSize-3 << " characters." << std::endl;
						isCaching = false;
						return;
					}
				}

				try
				{
					{
						std::unique_lock<std::mutex>slock(sprintfMut);
						sprintf (fileName, "%s%d.soscache", pathToCachingFiles.c_str(), fileNo++);
					}
					std::ofstream outFile;
					outFile.open (fileName);
					outFile << inputQueue.dequeue().c_str();
					outFile.close();

					std::unique_lock<std::mutex>lock(cacheFilesMut);
					cacheFiles.push_back(fileName);

				}
				catch (std::exception & e)
				{
					std::cout << "cache: " << e.what() << std::endl;
					isCaching = false;
				}
				catch (...)
				{
					std::cout << "cache: unexpected exception" << std::endl;
					isCaching = false;
				}
			}
			isCaching = false;
		}

		void CachingStream::delPastFile()
		{
			std::thread delPastFileThread(&MobileNoise::SOS::CachingStream::delPastFileTh, this);

			if (delPastFileThread.joinable())
				delPastFileThread.detach();
		}

		void CachingStream::delPastFileTh()
		{
			int ret = 0;
			std::unique_lock<std::mutex>lock(backedFilesMut);
			if (!backedFiles.empty())
			{
				ret = remove(backedFiles[0].c_str());
				if (ret != 0)
				{
					std::cout << "Deleting file " << backedFiles[0].c_str() << " failed with ERROR: " << ret << std::endl;
				}
				
					backedFiles.erase(backedFiles.begin());
					pastFiles = backedFiles.size();
				
			}
		}

	} // namespace SOS
} // namespace MobileNoise