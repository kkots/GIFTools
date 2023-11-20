#pragma once
#include <stdio.h>
#include <stdbool.h>

struct GIFDuration_response {
	size_t frame_count; // -1 - unknown or interrupted (reached end of range).
	int modifications_count; // 0 if none or error
	int error; // 0 if no error. -1 - invalid format.
};

struct GIFDuration_response GIFDuration_walker(FILE* file, int (*callback)(int), bool readOnly);

int changeGIFDurationRange_callback(int unused);

struct GIFDuration_response changeGIFDurationRange(FILE* file, int range_start, int range_end, int duration);

int changeGIFDurationFile_callback(int unused);

struct GIFDuration_response changeGIFDurationFile(FILE* file, FILE* durationFile);

void reportGIFDuration_output(int const prevFrame, int const prevDuration, const int currentFrame);

int reportGIFDuration_callback(int duration);

int reportGIFDuration(FILE* file);

int reportGIFDurationDurationsFormat_callback(int duration);

int reportGIFDurationDurationsFormat(FILE* file);
