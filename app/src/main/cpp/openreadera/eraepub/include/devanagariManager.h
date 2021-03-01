/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Developers: ReadEra Team (2013-2020), Tarasus (2018-2020).
 */
 //
 // Created by Tarasus on 26.06.2020.
 //

#ifndef _OPENREADERA_DEVANAGARI_MANAGER_H_
#define _OPENREADERA_DEVANAGARI_MANAGER_H_

#include <map>
#include <string>
#include <vector>
#include "lvtinydom.h"
#include "dvngLig.h"

std::vector <std::string> parse(const std::string &s, char delimeter);



typedef std::map<lChar16 , dvngLig> DvngLigMap;
typedef std::map<dvngLig , lChar16, Comparator> DvngLigMapRev;
typedef std::map<lUInt32 , int> DvngFastLigMap;

extern DvngLigMap     gDvngLigMap;
extern DvngLigMapRev  gDvngLigMapRev;
extern DvngFastLigMap gDvngFastLigMap;

DvngLigMap DevanagariLigMap();

DvngLigMapRev DevanagariLigMapReversed();

lChar16 findDvngLigRev(dvngLig combo);

dvngLig findDvngLig(lChar16 ligature);

lChar16 findDvngLigGlyphIndex(lChar16 ligature);

std::vector<std::string> parse(const std::string& s, char delimeter);

lString16 restoreDvngWord(lString16 in);

void parseFontIndexes();

#endif //_OPENREADERA_DEVANAGARI_MANAGER_H_
