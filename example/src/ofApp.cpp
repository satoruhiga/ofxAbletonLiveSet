#include "ofMain.h"

#include "ofxAbletonLiveSet.h"

class ofApp : public ofBaseApp
{
public:
	void setup()
	{
		ofxAbletonLiveSet::LiveSet LS;
		
		ofxAbletonLiveSet::Parser parser(LS);
		assert(parser.open("test.als"));
		
		for (int i = 0; i < LS.miditracks.size(); i++)
		{
			const ofxAbletonLiveSet::MidiTrack &T = LS.miditracks.at(i);
			cout << "Track Name: " << T.name << endl;
			
			for (int j = 0; j < T.clips.size(); j++)
			{
				const ofxAbletonLiveSet::MidiClip& m = T.clips.at(j);
				cout << "Clip Name: " << m.name << endl;
				cout << "Clip Start Time: " << m.time << endl;
				cout << "Clip Duration: " << m.duration << endl;
				
				for (int k = 0; k < m.notes.size(); k++)
				{
					cout << "Note Start Time: " << m.notes[k].time << endl;
				}
				
				for (int k = 0; k < m.envelopes.size(); k++)
				{
					cout << "Envelope ID: " << m.envelopes[k].id << endl;
				}
			}
			
			cout << "===" << endl << endl;
		}
		
		for (int i = 0; i < LS.locators.size(); i++)
		{
			const ofxAbletonLiveSet::Locator &L = LS.locators.at(i);
			cout << "Locator Name: " << L.name << endl;
			cout << "Locator Time: " << L.time << endl;
		}
		
		ofExit();
	}
	
	void update()
	{
	}
	
	void draw()
	{
	}

	void keyPressed(int key)
	{
	}

	void keyReleased(int key)
	{
	}
	
	void mouseMoved(int x, int y)
	{
	}

	void mouseDragged(int x, int y, int button)
	{
	}

	void mousePressed(int x, int y, int button)
	{
	}

	void mouseReleased(int x, int y, int button)
	{
	}
	
	void windowResized(int w, int h)
	{
	}
};


int main(int argc, const char** argv)
{
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp);
	return 0;
}
