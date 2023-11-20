#include "GIF_parse.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "CrossPlatformDefs.h"

/**
* Function walks a GIF file and calls the callback function whenever it finds a frame duration segment,
* so it can modify or report the frame durations.
* In the end reports GIF's number of frames.
* Has two modes: readonly and write. In readonly mode it sends the actual GIF duration to the callback but can't modify the file.
* In write mode it doesn't know frame durations so it sends -1 to the callback, but it can modify.
* Return -1 from the callback to not modify the duration of that particular frame.
* Return -2 from the callback to exit the file prematurely.
*/
struct GIFDuration_response GIFDuration_walker(FILE* file, int (*callback)(int), bool readOnly)
{
	struct GIFDuration_response response;
	response.frame_count = -1;
	response.modifications_count = 0;
	response.error = 0;

	int frame_count = 0;
	char stringbuf[4];
	int c;
	char hasGlobalColorMap = 0;
	char bitsPerPixel = 0;
	if (fgets(stringbuf, 4, file) == NULL) {
		response.frame_count = -1;
		response.error = -1;
		return response;
	}
	if (strcmp(stringbuf, "GIF") != 0) {
		response.frame_count = -1;
		response.error = -1;
		return response;
	}
	fseek(file, 7, SEEK_CUR); // skip 3 bytes GIF version and 4 bytes screen size
	c = fgetc(file);
	if (c == EOF) {
		response.frame_count = -1;
		response.error = -1;
		return response;
	}
	if ((c & 0x80) == 0x80) {
		hasGlobalColorMap = 1;
		bitsPerPixel = (c & 0x07) + 1;
	}

	fseek(file, 2, SEEK_CUR); // skip bytes 6-7 of Screen descriptor

	// skip Global Color Map
	if (hasGlobalColorMap) {
		fseek(file,
			(1 << (bitsPerPixel)) * 3,
			SEEK_CUR);
	}

	while (true) {
		c = fgetc(file);
		if (c == EOF) {
			response.frame_count = -1;
			response.error = -1;
			return response;
		}
		if (c == 0x3B) { // GIF Trailer
			return response;
		}

		if (c == 0x21) { // some kind of extension
			c = fgetc(file);
			if (c == EOF) {
				response.frame_count = -1;
				response.error = -1;
				return response;
			}
			if (c == 0xF9) {
				// Graphic Control Extension
				frame_count += 1;
				response.frame_count = frame_count;

				fseek(file, 2, SEEK_CUR);
				int callbackResult = 0;
				if (!readOnly) {
					if (callback == NULL) {
						callbackResult = -1;
					}
					else {
						callbackResult = callback(-1);
					}
					if (callbackResult >= 0) {
						fwrite(&callbackResult, 2, 1, file);
						++response.modifications_count;
						fseek(file, 2, SEEK_CUR);
					}
					else if (callbackResult != -2) {
						fseek(file, 4, SEEK_CUR);
					}
				}
				else if (callback != NULL) {
					int duration = 0;
					fread(&duration, 2, 1, file);
					callbackResult = callback(duration);
					fseek(file, 2, SEEK_CUR);
				}
				else {
					fseek(file, 4, SEEK_CUR);
				}
				if (callbackResult == -2) {
					response.frame_count = -1;
					return response; // premature successful exit
				}
			}
			else {
				// Other type of extension, usually denoted by length of header information, then a chain of data blocks terminated by 0
				c = fgetc(file);
				if (c == EOF) {
					response.frame_count = -1;
					response.error = -1;
					return response;
				}
				fseek(file, c, SEEK_CUR);
				while (true) {
					c = fgetc(file);
					if (c == EOF) {
						response.frame_count = -1;
						response.error = -1;
						return response;
					}
					if (c == 0) {
						break;
					}
					fseek(file, c, SEEK_CUR);
				}
			}
		}
		else if (c == 0x2C) { // Image Descriptor
			fseek(file, 8, SEEK_CUR);
			char hasLocalColorTable = 0;
			bitsPerPixel = 0;
			c = fgetc(file);
			if (c == EOF) {
				response.frame_count = -1;
				response.error = -1;
				return response;
			}
			if ((c & 0x80) == 0x80) {
				hasLocalColorTable = 1;
				bitsPerPixel = (c & 0x07) + 1;
			}
			if (hasLocalColorTable) {
				fseek(file,
					(1 << (bitsPerPixel)) * 3,
					SEEK_CUR);
			}

			// Table Based Image Data
			fseek(file, 1, SEEK_CUR); // LZW Minimum Code Size
			while (true) {
				c = fgetc(file);
				if (c == EOF) {
					response.frame_count = -1;
					response.error = -1;
					return response;
				}
				if (c == 0) {
					break;
				}
				fseek(file, c, SEEK_CUR);
			}
		}
		else {
			response.frame_count = -1;
			response.error = -1;
			return response;
		}
	}

	response.frame_count = -1;
	response.error = -1;
	return response;
}

int changeGIFDurationRange_range_start; // won't change throughout
int changeGIFDurationRange_range_end; // won't change throughout
int changeGIFDurationRange_duration; // won't change throughout
int changeGIFDurationRange_durationDividedBy10; // won't change throughout
char changeGIFDurationRange_durationRemainderBy10; // won't change throughout
char changeGIFDurationRange_durationRemainder;
int changeGIFDurationRange_frame_count;

int changeGIFDurationRange_callback(int unused) {
	++changeGIFDurationRange_frame_count;
	if (changeGIFDurationRange_frame_count - 1 > changeGIFDurationRange_range_end) {
		return -2;
	}
	if (changeGIFDurationRange_range_start <= changeGIFDurationRange_frame_count - 1
		&& changeGIFDurationRange_frame_count - 1 <= changeGIFDurationRange_range_end) {
		changeGIFDurationRange_durationRemainder += changeGIFDurationRange_durationRemainderBy10;
		int duration_LE = changeGIFDurationRange_durationDividedBy10 + changeGIFDurationRange_durationRemainder / 10;
		changeGIFDurationRange_durationRemainder = changeGIFDurationRange_durationRemainder % 10;
		return duration_LE;
	}
	return -1;
}

/**
 * Function modifies durations in a GIF file in a given range of frames.
 * @param file GIF file
 * @param range_start Starting from 0 the range which is to be modified
 * @param range_end Starting from 0 the range which is to be modified
 * @param duration In milliseconds. The duration to be set for the range being modified.
*/
struct GIFDuration_response changeGIFDurationRange(FILE* file, int range_start, int range_end, int duration) {
	changeGIFDurationRange_range_start = range_start; // won't change throughout
	changeGIFDurationRange_range_end = range_end; // won't change throughout
	changeGIFDurationRange_duration = duration; // won't change throughout
	changeGIFDurationRange_durationDividedBy10 = duration / 10; // won't change throughout
	changeGIFDurationRange_durationRemainderBy10 = duration % 10; // won't change throughout
	changeGIFDurationRange_durationRemainder = 0;
	changeGIFDurationRange_frame_count = 0;

	return GIFDuration_walker(file, changeGIFDurationRange_callback, false);
}

FILE* changeGIFDurationFile_file;
int changeGIFDurationFile_frameCount;
bool changeGIFDurationFile_error;
char changeGIFDurationFile_durationRemainder;

int changeGIFDurationFile_callback(int unused) {
#define SETTINGS_PROPERTY_LENGTH 8
	++changeGIFDurationFile_frameCount;

	char textline[SETTINGS_PROPERTY_LENGTH];
	if (fgets(textline, SETTINGS_PROPERTY_LENGTH, changeGIFDurationFile_file) == NULL) {
		if (feof(changeGIFDurationFile_file) != 0) {
			CrossPlatformCerr << CrossPlatformText("Reached end of durations file before reaching end of GIF.\n");
		} else {
			CrossPlatformPerror(NULL);
			CrossPlatformCerr << CrossPlatformText("Failed to read text from durations file on frame ") << changeGIFDurationFile_frameCount << std::endl;
		}
		changeGIFDurationFile_error = true;
		return -2;
	}
	#ifndef FOR_LINUX
	size_t str_size = strnlen_s(textline, SETTINGS_PROPERTY_LENGTH);
	#else
	size_t str_size = 0;
	char* textlinePtr = textline;
	while (str_size + 1 < SETTINGS_PROPERTY_LENGTH) {
		if (*textlinePtr == '\0') break;
		++textlinePtr;
		++str_size;
	}
	#endif
	if (str_size == SETTINGS_PROPERTY_LENGTH - 1  && feof(changeGIFDurationFile_file) == 0 && textline[str_size - 1] != '\n') {
		CrossPlatformCerr << CrossPlatformText("String on line ") << changeGIFDurationFile_frameCount
			<< CrossPlatformText(" exceeds ") << SETTINGS_PROPERTY_LENGTH - 2 << CrossPlatformText(" characters in durations file.\n");
		changeGIFDurationFile_error = true;
		return -2;
	}

	if (textline[str_size - 1] == '\n') {
		textline[str_size - 1] = '\0';
		str_size--;
	}
	if (textline[str_size - 1] == '\r') {  // text-mode? What is text mode?
		textline[str_size - 1] = '\0';
		str_size--;
	}

	if (str_size == 0) return -1;
	char c;
	char* cptr = textline;
	while ((c = *cptr) != '\0') {
		if (!(c >= '0' && c <= '9')) {
			CrossPlatformCerr << CrossPlatformText("Durations file contains invalid characters on frame ") << changeGIFDurationFile_frameCount << std::endl;
			changeGIFDurationFile_error = true;
			return -2;
		}
		++cptr;
	}

	int duration = atoi(textline);
	int durationDividedBy10 = duration / 10;
	int durationNewRemainder = duration % 10;
	changeGIFDurationFile_durationRemainder += durationNewRemainder;
	if (changeGIFDurationFile_durationRemainder >= 10) {
		durationDividedBy10 += 1;
		changeGIFDurationFile_durationRemainder -= 10;
	}
	return durationDividedBy10;


#undef SETTINGS_PROPERTY_LENGTH
}

/**
 * Function modifies durations in a GIF file by taking duration values from a file.
 * File must contain duration in ms on each line in ASCII encoding. Only numbers and newlines allowed.
*/
struct GIFDuration_response changeGIFDurationFile(FILE* file, FILE* durationFile) {
	changeGIFDurationFile_error = false;
	changeGIFDurationFile_frameCount = 0;
	changeGIFDurationFile_durationRemainder = 0;
	changeGIFDurationFile_file = durationFile;

	struct GIFDuration_response res = GIFDuration_walker(file, changeGIFDurationFile_callback, false);

	if (changeGIFDurationFile_error) {
		res.error = -1;
	}
	return res;
}

int reportGIFDuration_prevDuration;
int reportGIFDuration_prevFrame;
int reportGIFDuration_currentFrame;
int reportGIFDuration_durationSum;
int reportGIFDuration_duration;

void reportGIFDuration_output(int const prevFrame, int const prevDuration, const int currentFrame)
{
	if (currentFrame == prevFrame + 1) {
		CrossPlatformCout << prevFrame  << CrossPlatformText(": ") << prevDuration * 10 << CrossPlatformText(" ms (")
			<< (int)round(100. / prevDuration) << CrossPlatformText(" fps)\n");
	}
	else {
		CrossPlatformCout << prevFrame << "-" << currentFrame - 1 << CrossPlatformText(": ")
			<< prevDuration * 10 << " ms (" << (int)round(100. / prevDuration) << CrossPlatformText(" fps)\n");
	}
}

int reportGIFDuration_callback(int duration) {
	reportGIFDuration_currentFrame++;
	if (reportGIFDuration_prevFrame == -1) {
		reportGIFDuration_prevDuration = duration;
		reportGIFDuration_prevFrame = reportGIFDuration_currentFrame;
	}
	else if (duration != reportGIFDuration_prevDuration) {
		reportGIFDuration_output(reportGIFDuration_prevFrame, reportGIFDuration_prevDuration, reportGIFDuration_currentFrame);
		reportGIFDuration_prevDuration = duration;
		reportGIFDuration_prevFrame = reportGIFDuration_currentFrame;
	}
	reportGIFDuration_durationSum += duration;
	return 0;
}

/**
 * Function prints to console: frame ranges which have constant framerate, their durations and framerate.
 * Returns error code. 0 for no error.
 * @param file GIF file
*/
int reportGIFDuration(FILE* file)
{
	reportGIFDuration_prevDuration = 0;
	reportGIFDuration_prevFrame = -1;
	reportGIFDuration_currentFrame = 0;
	reportGIFDuration_durationSum = 0;
	reportGIFDuration_duration = 0;

	struct GIFDuration_response res = GIFDuration_walker(file, reportGIFDuration_callback, true);
	if (res.error != 0) {
		return -1;
	}

	reportGIFDuration_output(reportGIFDuration_prevFrame, reportGIFDuration_prevDuration, reportGIFDuration_currentFrame + 1);
	CrossPlatformCout << CrossPlatformText("Average duration: ")
		<< (int)round(
			(double)reportGIFDuration_durationSum / reportGIFDuration_currentFrame * 10
		)  << CrossPlatformText(" ms\nAverage framerate: ")
		<< (int)round(
			100. / reportGIFDuration_durationSum * reportGIFDuration_currentFrame
		) << CrossPlatformText(" fps\n");

	return 0;

}

int reportGIFDurationDurationsFormat_callback(int duration) {
	CrossPlatformCout << duration * 10 << std::endl;
	return 0;
}

/**
 * Function prints to console: frame ranges which have constant framerate, their durations and framerate.
 * Returns error code. 0 for no error.
 * @param file GIF file
*/
int reportGIFDurationDurationsFormat(FILE* file)
{
	struct GIFDuration_response res = GIFDuration_walker(file, reportGIFDurationDurationsFormat_callback, true);
	if (res.error != 0) {
		return -1;
	}

	return 0;

}
