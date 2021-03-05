/*
The MIT License (MIT)

Copyright (c) <2021> <Amber Bhardwaj>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "decoder.h"
#include <sstream>
#include <fstream>
using namespace std;


//// Example taken from official FAA website
//Input: !PHX 12/133 PHX RWY 07R/25L EDGE MARKINGS NOT STD 2001010700-2101010659
//------
//Output: 
//-------
//Issuing Airport:	(PHX) Phoenix Sky Harbor Intl 
//NOTAM Number:	12/133
//Effective Time Frame
//Beginning:	Wednesday, January 1, 2020 0700 (UTC)
//Ending:	Friday, January 1, 2021 0659 (UTC)
//Affected Areas
//Runway:	07R/25L
//Marking Location:	Edge Markings
//Marking Type:	Non-Standard 
//
//-------------------------------------------------------------------
//Conponent Break down:
//=====================
//
//E.g.  "!ABC 07/003 XYZ NAV VOR OTS WEF 0407141200-0407162000"
//
//! is the ADP Code*. The national NOTAM computer needs this
//symbol to process the NOTAM. (*ADP = Automatic Data
//Processing).
//
//ABC is the identifier of the accountability location, which is used
//by the NOTAM system computer to keep track of NOTAM numbering.
//
//07/003 - The 07 represents the month the NOTAM is issued; e.g.
//07 is July. The 003 indicates that this is the third NOTAM issued
//under the ABC accountability location for the month of July.
//
//XYZ is the identifier of the affected facility or location.
//
//NAV is the NOTAM keyword indicating “Navigation Aids”
//
//VOR is, in this example, the affected facility.
//
//OTS is, in this example, the condition of the affected facility.
//
//WEF 0407141200-0407162000 is the effective time of the
//abnormal condition of the affected facility or location. WEF is the
//acronym for effective from (or with effect from).
//
//
//
//-------------------------------------------------------------------
//https://www.faa.gov/news/safety_briefing/2008/media/janfeb2008.pdf


void CFaaNotam::openFile(string &msg, int &nextMsgLocation) // Use this function for debugging
{
	ifstream myfile("notam_faa_messages.txt");
	int newline = 0;
	if (myfile.is_open())
	{
		char c;
		while (myfile.good())
		{
			while (myfile.get(c) )
			{
				if (c == '!') //Increment whenver you find twice the "!" mark
				{
					newline++;
				}
				if (newline == 1)
				{
					msg.append(1,c);
				}
				else
				{
					newline = 1;
					nextMsgLocation = static_cast<int>(myfile.tellg());
					// Code to Display and Decode Msgs
					stNotamDataInternals temp;
					init(msg);
					splitNotamMsgIntoKeywords();
					decoder(temp);
					display(temp);
					msg.clear();
					msg.append(1, c); // Copied the "!" mark;
				}
				
			}
		}
	}
	myfile.close();
}

void CFaaNotam::decoder(stNotamDataInternals &data)
{
	// use the splitted strings then check on by one for abbrivations and assign it to data;
	if (keywords.empty() || keywords[0][0] != '!' )
	{
		return;
	}

	int fixedKeywordsCount = 4;
	std::unordered_map<std::string, std::string>::const_iterator itr;
	
	//Accountability location
	data._issuingAirport				= keywords[0].substr(1, keywords[0].length());
	// Month/Number of the NOTAM is issued;
	data._notamNumber				= keywords[1];
	// Affected facility or location
	data._affectedLocation			= keywords[2];
	// NOTAM keyword
	itr = SubjectLookup.find(keywords[3]);
	if (itr != SubjectLookup.end())
	{
		data._aerodromeOrNotamKeyword = itr->second;
	}
	else
	{
		data._aerodromeOrNotamKeyword = keywords[3];
	}

	// Last element in vector will be related to Dates and the second last element 
	// may or may not be related to Dates until unless that element contains "WEF"
	// So we're going to check second last element,  first so that we can define
	// the processing size in a vector to fetch the data.

	if (keywords[keywords.size() - 2] == "WEF")
	{
		fixedKeywordsCount = keywords.size() - 2; // 2 because one is for WEF and another for combined dates
	}
	else if (keywords[keywords.size() - 1].length() > 8) // Magic number because we expect date string here e.g. 2001010700-2101010659
		fixedKeywordsCount = keywords.size() - 1;
	else
		fixedKeywordsCount = keywords.size();
	
	for (int i = 4; i < fixedKeywordsCount; i++)
	{
		itr = SubjectLookup.find(keywords[i]);
		if (itr != SubjectLookup.end())
		{
			data._msg += itr->second + " ";
		}
		else
			data._msg += keywords[i] + " ";
	}
	// Extracting the date information
	if (keywords[keywords.size() - 1].length() > 8) // Magic number because we expect date string here e.g. 2001010700-2101010659
		effectiveTimeDecoder(keywords[keywords.size()-1], data._beginning, data._ending);
}

void CFaaNotam::splitNotamMsgIntoKeywords(void)
{
	if (notamMsg.length() > 0)
	{
		istringstream ss(notamMsg);
		while(ss)
		{
			std::string word;
			//gte each word parsed from string stream
			ss >> word;
//			cout << word << "\n";
			if (word.length() && word != "\"" )
			keywords.emplace_back(word);
		}
	}
}
//This function is used to get the "When in Effect".
//The (WEF) time includes both a “start” set and an “ending” set. The digits 
//in each pair always appear in the following:
//[order: Year(2 digits) – month(2 digits) – day(2 digits) – Zulu(UTC) time(4 digits)]

void CFaaNotam::effectiveTimeDecoder ( std::string s, std::string & Beginning, std::string & Ending )
{
	// Length for effective date string is always fixed e.g. [YearMonthDayZuluUTCTimeBegin-YearMonthDayZuluUTCTimeEnd]
	// The string will always be 20bytes long + 1 byte for '-'

#define NOTAM_WHEN_IN_EFFECT_TOTAL_LEN	(21) //length = Begin-End
#define NOTAM_WHEN_IN_EFFECT_DATE_LEN	(NOTAM_WHEN_IN_EFFECT_TOTAL_LEN - 10 -1)
	if (!s.empty())
	{
		// Remove the trailing space
		for (int i = 0; i < s.length(); i++)
		{
			if ((s[i] == '"' || s[i] == '\n' || s[i] == ' ' || s[i] == '\r'))
			{
				s[i] = '\0';
			}
		}
	}
	if ( (s.length() == NOTAM_WHEN_IN_EFFECT_TOTAL_LEN) || (s.length() == NOTAM_WHEN_IN_EFFECT_DATE_LEN) )
	{
		// Calculate the NOTAM's Beginning date e.g. 19 12 19 1835 => Thursday, December 19, 2019 1835 (UTC)

		std::size_t pos;
		std::string year, month, day, utc;

		
		//[order: Year(2 digits) – month(2 digits) – day(2 digits) – Zulu(UTC) time(4 digits)]
		if ((s.length() == NOTAM_WHEN_IN_EFFECT_TOTAL_LEN))
		{
			pos = s.find('-');
			Beginning = s.substr(0, pos);
		}
		else
		{
			Beginning = s;
		}

		year	= "20" + Beginning.substr(0, 2);
		month	= Beginning.substr(2, 2);
		day		= Beginning.substr(4, 2);
		utc = Beginning.substr(6, 4);
		Beginning.clear();
		if (!((atoi(month.c_str()) > 12) || (atoi(day.c_str()) > 31)))
		{
			Beginning = getTheDayOfTheWeek(atoi(year.c_str()), atoi(month.c_str()), atoi(day.c_str())) + ", "
				+ months[atoi(month.c_str()) - 1] + " "
				+ day + ", "
				+ year + "  "
				+ utc + " (UTC)";
		}

		if (s.length() == NOTAM_WHEN_IN_EFFECT_TOTAL_LEN)
		{
			// Clearing the memories
			year.clear();
			month.clear();
			day.clear();
			utc.clear();

			// Calculate the NOTAM's Ending date e.g. 2004010659 => Wednesday, April 1, 2020 0659 (UTC)
			Ending = s.substr(pos + 1, s.length() - pos);
			year = "20" + Ending.substr(0, 2);
			month = Ending.substr(2, 2);
			day = Ending.substr(4, 2);
			utc = Ending.substr(6, 4);
			Ending.clear();

			if (!((atoi(month.c_str()) > 12) || (atoi(day.c_str()) > 31)))
			{
				Ending = getTheDayOfTheWeek(atoi(year.c_str()), atoi(month.c_str()), atoi(day.c_str())) + ", "
					+ months[atoi(month.c_str()) - 1] + " "
					+ day + ", "
					+ year + "  "
					+ utc + " (UTC)";
			}
		}
	}
	//return empty strings if input length is not equal to standard format length
}

std::string CFaaNotam::getTheDayOfTheWeek(int y, int m, int d)
{
	//The term [2.6m − 0.2] mod 7 gives the values of months : t
	int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	y -= m < 3;
	int day =  (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
	
	return days.at(day);
}

std::string CFaaNotam::getNotamMsg(void)
{
	return notamMsg;
}

void CFaaNotam::init(const std::string &msg)
{
	notamMsg.clear();
	keywords.clear();
	aerodromeMapping.clear();
	notamMsg = msg;
}
void CFaaNotam::display(const stNotamDataInternals &data)
{
	cout << "Issue Airport:		" << data._issuingAirport << endl;
	cout << "NOTAM Number:		" << data._notamNumber << endl;
	if (!data._beginning.empty() || !data._ending.empty())
	{
		cout << "\nEffective Time Frame" << endl;
		if (!data._beginning.empty())
			cout << "Beginning:		" << data._beginning << endl;
		if (!data._ending.empty())
			cout << "Ending:			" << data._ending << endl;
	}
	cout << "\nAffected Areas" << endl;
	cout << "Section:		" << data._aerodromeOrNotamKeyword << endl;
	cout << "Information:		" << data._msg<< endl;
	cout << "--------------------------------------------------------------------------------------" << endl;
	// Add more param to display things
}

#include <memory>
int main (){
	CFaaNotam &b = CFaaNotam::getInstanse();
	
	stNotamDataInternals temp;
	string notamMsg;
	int lastLoc = 0;
	b.openFile(notamMsg, lastLoc);
	//b.init("!PHX 12/133 PHX RWY 07R/25L EDGE MARKINGS NOT STD WEF 2001010700-2101010659 \t");
	//b.init("!ABC 07/003 XYZ NAV VOR OTS WEF 0407141200-0407162000");
	//b.init("!XYZ 01/008 XYZ TWY B CLSD");
	//b.init("!XYZ 08/005 XYZ OBST TOWER 580 (420 AGL) 5 NE LGTS OTS (ASR 1068993) TIL 0802091500");
	//b.init("!ABC 04/002 XYZ RWY 18/36 CMSND 4500X75 ASPH/LGTD");
	//b.splitNotamMsgIntoKeywords();
	//b.decoder(temp);
	//b.display(temp);
}
