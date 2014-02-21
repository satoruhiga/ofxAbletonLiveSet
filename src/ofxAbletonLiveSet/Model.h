#pragma once

#include "Constants.h"
#include "Time.h"

OFX_ALS_BEGIN_NAMESPACE

typedef float Time;

struct Note
{
	Time time;
	Time duration;
	float velocity;
	int key;
};

struct MidiClip
{
	Time time;
	Time endtime;
	Time duration;
	
	string name;
	string annotation;
	int color;
	
	vector<Note> notes;
};

struct Track
{
	string name;
	int color;
};

struct MidiTrack : public Track
{
	vector<MidiClip> clips;
};

struct Locator
{
	Time time;
	string name;
	string annotation;
};

struct LiveSet
{
	Tempo tempo;
	vector<Locator> locators;
	vector<MidiTrack> miditracks;
};

OFX_ALS_END_NAMESPACE
