#pragma once

#include <Poco/InflatingStream.h>

#include "Constants.h"
#include "Tempo.h"
#include "Model.h"

OFX_ALS_BEGIN_NAMESPACE

class Parser
{
public:
	
	Parser(LiveSet &LS) : LS(LS) {}
	
	bool open(const string& path)
	{
		ifstream ifs;
		ifs.open(ofToDataPath(path).c_str());
		if (!ifs) return false;
		
		Poco::InflatingInputStream inflater(ifs, Poco::InflatingStreamBuf::STREAM_GZIP);
		if (!inflater) return false;
		
		pugi::xml_document doc;
		if (!doc.load(inflater)) return false;

		parseTempo(doc);
		parseMidiTrack(doc);
		parseLocator(doc);
		
		return true;
	}
	
private:
	
	LiveSet &LS;
	int controller_target_offset;
	
	void parseTempo(const pugi::xml_document& doc)
	{
		pugi::xpath_query q("//Tempo");
		pugi::xpath_node_set nodes = q.evaluate_node_set(doc);
		
		if (!nodes.empty())
		{
			parse(LS.tempo, nodes[0].node());
		}
	}
	
	void parse(Tempo& tempo, const pugi::xml_node& node)
	{
		map<BarTime, float> events;
		
		pugi::xpath_query q("ArrangerAutomation/Events/FloatEvent");
		pugi::xpath_node_set nodes = q.evaluate_node_set(node);
		
		for (int i = 0; i < nodes.size(); i++)
		{
			const pugi::xml_node& n = nodes[i].node();
			
			BarTime time = n.attribute("Time").as_float();
			float value = n.attribute("Value").as_float();
			events[time] = value;
		}
		
		float default_tempo = node.child("Manual").attribute("Value").as_float();
		tempo.updateTempoMap(events, default_tempo);
	}
	
	//
	
	void parseMidiTrack(const pugi::xml_document& doc)
	{
		pugi::xpath_query q("//Tracks/MidiTrack");
		pugi::xpath_node_set nodes = q.evaluate_node_set(doc);
		
		LS.miditracks.clear();
		
		for (int i = 0; i < nodes.size(); i++)
		{
			MidiTrack MT;
			parse(MT, nodes[i].node(), 0);
			LS.miditracks.push_back(MT);
		}
	}

	void parse(MidiTrack& MT, const pugi::xml_node &node, RealTime offset)
	{
		MT.name = node.child("Name").child("EffectiveName").attribute("Value").value();
		MT.color = node.child("ColorIndex").attribute("Value").as_int();
		
		{
			pugi::xpath_query q(".//MidiControllers/ControllerTargets.0");
			pugi::xpath_node_set nodes = q.evaluate_node_set(node);
			
			if (nodes.size())
			{
				controller_target_offset = nodes[0].node().attribute("Id").as_int(0);
			}
		}
		
		pugi::xpath_query q(".//ArrangerAutomation/Events/MidiClip");
		pugi::xpath_node_set nodes = q.evaluate_node_set(node);
		
		MT.clips.clear();
		
		for (int i = 0; i < nodes.size(); i++)
		{
			MidiClip MC;
			parse(MC, nodes[i].node(), 0);
			MT.clips.push_back(MC);
		}
		
		std:sort(MT.clips.begin(), MT.clips.end(), sort_by_time<MidiClip>);
	}
	
	void parse(Note& note, const pugi::xml_node &node, RealTime offset)
	{
		const pugi::xml_node& e = node;
		note.time = LS.tempo.toRealTime(e.attribute("Time").as_float(0)) + offset;
		note.duration = LS.tempo.toRealTime(e.attribute("Duration").as_float(0));
		note.velocity = e.attribute("Velocity").as_float(0);
	}

	void parse(MidiClip& MC, const pugi::xml_node &node, RealTime offset)
	{
		float start = node.child("CurrentStart").attribute("Value").as_float();
		float end = node.child("CurrentEnd").attribute("Value").as_float();
		
		MC.time = LS.tempo.toRealTime(start) + offset;
		MC.endtime = LS.tempo.toRealTime(end) + offset;
		MC.duration = MC.endtime - MC.time;
		
		MC.color = node.child("ColorIndex").attribute("Value").as_int();
		MC.name = node.child("Name").attribute("Value").value();
		MC.annotation = node.child("Annotation").attribute("Value").value();
		
		{
			pugi::xpath_query q("Notes//KeyTracks//KeyTrack");
			pugi::xpath_node_set nodes = q.evaluate_node_set(node);
			
			MC.notes.clear();
			
			for (int i = 0; i < nodes.size(); i++)
			{
				const pugi::xml_node& key_tracks = nodes[i].node();
				int midi_key = key_tracks.child("MidiKey").attribute("Value").as_int();
				
				pugi::xml_object_range<pugi::xml_node_iterator> notes = key_tracks.child("Notes").children();
				
				pugi::xml_node_iterator it = notes.begin();
				while (it != notes.end())
				{
					Note note;
					note.key = midi_key;
					
					parse(note, *it, MC.time);
					MC.notes.push_back(note);
					
					it++;
				}
			}
			
			std:sort(MC.notes.begin(), MC.notes.end(), sort_by_time<Note>);
		}

		{
			pugi::xpath_query q("Envelopes//ClipEnvelope");
			pugi::xpath_node_set nodes = q.evaluate_node_set(node);
			
			for (int i = 0; i < nodes.size(); i++)
			{
				const pugi::xml_node& clip_envelope = nodes[i].node();
				
				int id = clip_envelope.child("EnvelopeTarget").child("PointeeId").attribute("Value").as_int() - controller_target_offset;
				
				MC.envelopes.push_back(Automation());
				Automation &automation = MC.envelopes.back();
				
				automation.id = id;
				
				pugi::xml_object_range<pugi::xml_node_iterator> events = clip_envelope.child("Automation").child("Events").children();
				pugi::xml_node_iterator it = events.begin();
				
				while (it != events.end())
				{
					float time = LS.tempo.toRealTime(it->attribute("Time").as_float()) + MC.time;
					float value = it->attribute("Value").as_float();
					if (time < 0) time = 0;
					
					automation.events[time] = value;
					it++;
				}
			}
		}
	}

	//

	void parseLocator(const pugi::xml_document& doc)
	{
		pugi::xpath_query q("//Locators/Locators/Locator");
		pugi::xpath_node_set nodes = q.evaluate_node_set(doc);
		
		LS.locators.clear();
		
		for (int i = 0; i < nodes.size(); i++)
		{
			Locator L;
			parse(L, nodes[i].node(), 0);
			LS.locators.push_back(L);
		}
		
		std:sort(LS.locators.begin(), LS.locators.end(), sort_by_time<Locator>);
	}
	
	void parse(Locator& L, const pugi::xml_node& node, RealTime offset)
	{
		L.name = node.child("Name").attribute("Value").as_string();
		L.annotation = node.child("Annotation").attribute("Value").as_string();
		L.time = LS.tempo.toRealTime(node.child("Time").attribute("Value").as_float()) + offset;
	}

	//
	
	template <typename T>
	static bool sort_by_time(const T& v0, const T& v1) { return v0.time < v1.time; }
};

OFX_ALS_END_NAMESPACE
