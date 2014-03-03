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

struct Automation
{
	int id;
	float getValueAt(float time) const;
	map<float, float> events;
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
	vector<Automation> envelopes;
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

///

float Automation::getValueAt(float time) const
{
	if (events.size() < 2) return -1;
	
	map<float, float>::const_iterator start = events.upper_bound(time);
	map<float, float>::const_iterator end = start;
	start--;
	
	if (start == events.begin()) return start->second;
	if (end == events.end()) return start->second;
	
	const float t = time - start->first;
	const float td = end->first - start->first;
	const float vd = end->second - start->second;

	return start->second + vd * (t / td);
}

OFX_ALS_END_NAMESPACE
