#include "dataset_operations.h"

/**
 * @brief Writes data to a stream.
 *
 * This function is a callback used by libcurl to write data to a stream. It is typically
 * used when downloading a file. The data to be written is provided by libcurl and the
 * stream to which it should be written is provided as an argument.
 *
 * @param ptr A pointer to the data to be written.
 * @param size The size of each unit of data to be written.
 * @param nmemb The number of units of data to be written.
 * @param stream The stream to which the data should be written.
 * @return The number of bytes actually written.
 */
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

/**
 * @brief Downloads a file from a given URL and saves it to a specified path.
 *
 * This function uses libcurl to download a file. If the file already exists at the
 * specified path, the download is skipped. If the file does not exist, it is created
 * and the content from the URL is written to it. If the download fails for any reason,
 * an error message is printed to the console.
 *
 * @param url The URL of the file to download.
 * @param file_path The path where the downloaded file should be saved.
 */
void download_file(const char *url, const char *file_path)
{
	std::ifstream file(file_path);
	if (file.good())
	{
		// File already exists
		std::cout << "Tar File already exists, skipping download.\n";
		return;
	}

	CURL *curl;
	FILE *fp;
	CURLcode res;

	curl = curl_easy_init();
	if (curl)
	{
		fp = fopen(file_path, "wb");
		if (fp == NULL)
		{
			std::cout << "Could not open file for writing: " << file_path << std::endl;
			return;
		}
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
	}
}

/**
 * @brief Extracts a tar file to a specified directory.
 *
 * This function uses the libarchive library to extract a tar file. The tar file and the
 * directory to which it should be extracted are provided as arguments. If the extraction
 * fails for any reason, an error message is printed to the console.
 *
 * @param tar_file_path The path to the tar file to be extracted.
 * @param extract_dir_path The path to the directory where the tar file should be extracted.
 */
void extract_tar_file(const char *file_path)
{
	std::string command = "tar -xzf ";
	command += file_path;

	FILE *pipe = popen(command.c_str(), "r");
	if (!pipe)
	{
		return;
	}
	pclose(pipe);

	// Remove the file after extracting it
	if (std::remove(file_path) != 0)
	{
		std::perror("Error deleting file");
	}
	else
	{
		std::cout << "Tar File successfully deleted.\n";
	}
}