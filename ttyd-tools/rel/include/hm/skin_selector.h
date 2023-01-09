#pragma once

void skinselector_init();
void skinselector_update();
const char* skinselector_filealloc_hook(const char* path);
void skinselector_loadexit_hook();

struct Skin {
	const char* name;
	int16_t icon;
	const char* description;
	const char* filePaths[4]; // in order: normal, reversed, battle, extra
} __attribute__((__packed__));