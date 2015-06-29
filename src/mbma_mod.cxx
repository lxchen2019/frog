/*
  $Id$
  $URL$

  Copyright (c) 2006 - 2015
  Tilburg University

  This file is part of frog.

  frog is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  frog is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      http://ilk.uvt.nl/software.html
  or send mail to:
      timbl@uvt.nl
*/

#include <cstdlib>
#include <string>
#include <set>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "timbl/TimblAPI.h"

#include "ucto/unicode.h"
#include "ticcutils/Configuration.h"
#include "frog/Frog.h"
#include "frog/mbma_mod.h"

using namespace std;
using namespace folia;
using namespace TiCC;

const long int LEFT =  6; // left context
const long int RIGHT = 6; // right context

Mbma::Mbma(LogStream * logstream):
  MTreeFilename( "dm.igtree" ),
  MTree(0),
  transliterator(0),
  filter(0),
  doDaring(false)
{
  mbmaLog = new LogStream( logstream, "mbma-" );
}

Mbma::~Mbma() {
  cleanUp();
  delete transliterator;
  delete filter;
  delete mbmaLog;
}

void Mbma::fillMaps() {
  //
  // this could be done from a configfile
  //
  // first the CELEX POS tag names
  //
  tagNames[CLEX::N] = "noun";
  tagNames[CLEX::A] = "adjective";
  tagNames[CLEX::Q] = "quantifier/numeral";
  tagNames[CLEX::V] = "verb";
  tagNames[CLEX::D] = "article";
  tagNames[CLEX::O] = "pronoun";
  tagNames[CLEX::B] = "adverb";
  tagNames[CLEX::P] = "preposition";
  tagNames[CLEX::Y] = "conjunction";
  tagNames[CLEX::I] = "interjection";
  tagNames[CLEX::X] = "unanalysed";
  tagNames[CLEX::Z] = "expression_part";
  tagNames[CLEX::PN] = "proper_noun";
  //
  // now the inflections
  iNames['X'] = "";
  iNames['s'] = "separated";
  iNames['e'] = "singular";
  iNames['m'] = "plural";
  iNames['d'] = "diminutive";
  iNames['G'] = "genitive";
  iNames['D'] = "dative";
  iNames['P'] = "positive";
  iNames['C'] = "comparative";
  iNames['S'] = "superlative";
  iNames['E'] = "suffix-e";
  iNames['i'] = "infinitive";
  iNames['p'] = "participle";
  iNames['t'] = "present_tense";
  iNames['v'] = "past_tense";
  iNames['1'] = "1st_person_verb";
  iNames['2'] = "2nd_person_verb";
  iNames['3'] = "3rd_person_verb";
  iNames['I'] = "inversed";
  iNames['g'] = "imperative";
  iNames['a'] = "subjunctive";
}

// BJ: dirty hack with fixed everything to read in tag correspondences
void Mbma::init_cgn( const string& dir ) {
  string line;
  string fn = dir + "cgntags.main";
  ifstream tc( fn );
  if ( tc ){
    while( getline( tc, line) ) {
      vector<string> tmp;
      size_t num = split_at(line, tmp, " ");
      if ( num < 2 ){
	*Log(mbmaLog) << "splitting '" << line << "' failed" << endl;
	throw ( runtime_error("panic") );
      }
      TAGconv.insert( make_pair( tmp[0], tmp[1] ) );
    }
  }
  else
    throw ( runtime_error( "unable to open:" + fn ) );
  fn = dir + "cgntags.sub";
  ifstream tc1( fn );
  if ( tc1 ){
    while( getline(tc1, line) ) {
      vector<string> tmp;
      size_t num = split_at(line, tmp, " ");
      if ( num == 2 )
	TAGconv.insert( make_pair( tmp[0], tmp[1] ) );
    }
  }
  else
    throw ( runtime_error( "unable to open:" + fn ) );
}

Transliterator *Mbma::init_trans( ){
  UErrorCode stat = U_ZERO_ERROR;
  Transliterator *t = Transliterator::createInstance( "NFD; [:M:] Remove; NFC",
						      UTRANS_FORWARD,
						      stat );
  if ( U_FAILURE( stat ) ){
    throw runtime_error( "initFilter FAILED !" );
  }
  return t;
}


bool Mbma::init( const Configuration& config ) {
  *Log(mbmaLog) << "Initiating morphological analyzer..." << endl;
  debugFlag = 0;
  string val = config.lookUp( "debug", "mbma" );
  if ( val.empty() ){
    val = config.lookUp( "debug" );
  }
  if ( !val.empty() ){
    debugFlag = TiCC::stringTo<int>( val );
  }
  val = config.lookUp( "daring", "mbma" );
  if ( !val.empty() )
    doDaring = TiCC::stringTo<bool>( val );
  val = config.lookUp( "version", "mbma" );
  if ( val.empty() ){
    version = "1.0";
  }
  else
    version = val;
  val = config.lookUp( "set", "mbma" );
  if ( val.empty() ){
    mbma_tagset = "http://ilk.uvt.nl/folia/sets/frog-mbma-nl";
  }
  else
    mbma_tagset = val;

  val = config.lookUp( "set", "tagger" );
  if ( val.empty() ){
    cgn_tagset = "http://ilk.uvt.nl/folia/sets/frog-mbpos-cgn";
  }
  else
    cgn_tagset = val;

  val = config.lookUp( "clex_set", "mbma" );
  if ( val.empty() ){
    clex_tagset = "http://ilk.uvt.nl/folia/sets/frog-mbpos-clex";
  }
  else
    clex_tagset = val;

  string charFile = config.lookUp( "char_filter_file", "mbma" );
  if ( charFile.empty() )
    charFile = config.lookUp( "char_filter_file" );
  if ( !charFile.empty() ){
    charFile = prefix( config.configDir(), charFile );
    filter = new Tokenizer::UnicodeFilter();
    filter->fill( charFile );
  }
  string tfName = config.lookUp( "treeFile", "mbma" );
  if ( tfName.empty() )
    tfName = "mbma.igtree";
  MTreeFilename = prefix( config.configDir(), tfName );
  fillMaps();
  init_cgn( config.configDir() );
  string dof = config.lookUp( "filter_diacritics", "mbma" );
  if ( !dof.empty() ){
    bool b = stringTo<bool>( dof );
    if ( b ){
      transliterator = init_trans();
    }
  }
  //Read in (igtree) data
  string opts = config.lookUp( "timblOpts", "mbma" );
  if ( opts.empty() )
    opts = "-a1";
  opts += " +vs -vf"; // make Timbl run quietly
  MTree = new Timbl::TimblAPI(opts);
  return MTree->GetInstanceBase(MTreeFilename);
}

void Mbma::cleanUp(){
  // *Log(mbmaLog) << "cleaning up MBMA stuff " << endl;
  delete MTree;
  MTree = 0;
  clearAnalysis();
}

vector<string> Mbma::make_instances( const UnicodeString& word ){
  vector<string> insts;
  for( long i=0; i < word.length(); ++i ) {
    if (debugFlag > 10)
      *Log(mbmaLog) << "itt #:" << i << endl;
    UnicodeString inst;
    for ( long j=i ; j <= i + RIGHT + LEFT; ++j ) {
      if (debugFlag > 10)
	*Log(mbmaLog) << " " << j-LEFT << ": ";
      if ( j < LEFT || j >= word.length()+LEFT )
	inst += '_';
      else {
	if (word[j-LEFT] == ',' )
	  inst += 'C';
	else
	  inst += word[j-LEFT];
      }
      inst += ",";
      if (debugFlag > 10)
	*Log(mbmaLog) << " : " << inst << endl;
    }
    inst += "?";
    insts.push_back( UnicodeToUTF8(inst) );
    // store res
  }
  return insts;
}

MBMAana::MBMAana( const Rule& r, bool daring ): rule(r) {
  brackets = rule.resolveBrackets( daring );
  rule.getCleanInflect();
}

UnicodeString MBMAana::getKey( bool daring ){
  if ( sortkey.isEmpty() ){
    UnicodeString tmp;
    if ( daring ){
      stringstream ss;
      ss << getBrackets() << endl;
      tmp = UTF8ToUnicode(ss.str());
    }
    else {
      vector<string> v = getMorph();
      // create an unique string
      for ( size_t p=0; p < v.size(); ++p ) {
	tmp += UTF8ToUnicode(v[p]) + "+";
      }
    }
    sortkey = tmp;
  }
  return sortkey;
}

void Rule::getCleanInflect() {
  // get the FIRST inflection and clean it up by extracting only
  //  known inflection names
  inflection = "";
  vector<RulePart>::const_iterator it = rules.begin();
  while ( it != rules.end() ) {
    if ( !it->inflect.empty() ){
      //      cerr << "x inflect:'" << it->inflect << "'" << endl;
      string inflect;
      for ( size_t i=0; i< it->inflect.length(); ++i ) {
	if ( it->inflect[i] != '/' ){
	  // check if it is a known inflection
	  //	  cerr << "x bekijk [" << it->inflect[i] << "]" << endl;
	  string inf = get_iName(it->inflect[i]);
	  if ( inf.empty() ){
	    //	    cerr << "added unknown inflection X" << endl;
	    inflect += "X";
	  }
	  else {
	    //	    cerr << "added known inflection " << it->inflect[i]
	    //	     	 << " (" << inf << ")" << endl;
	    inflect += it->inflect[i];
	  }
	}
      }
      //      cerr << "cleaned inflection " << inflect << endl;
      inflection = inflect;
      return;
    }
    ++it;
  }
}

#define OLD_STEP

#ifdef OLD_STEP
string find_class( unsigned int step,
		   const vector<string>& classes,
		   unsigned int nranal ){
  string result = classes[0];
  if ( nranal > 1 ){
    if ( classes.size() > 1 ){
      if ( classes.size() > step )
	result = classes[step];
      else
	result = "0";
    }
  }
  return result;
}

vector<vector<string> > generate_all_perms( const vector<string>& classes ){
  // determine all alternative analyses, remember the largest
  // and store every part in a vector of string vectors
  int largest_anal=1;
  vector<vector<string> > classParts;
  classParts.resize( classes.size() );
  for ( unsigned int j=0; j< classes.size(); ++j ){
    vector<string> parts;
    int num = split_at( classes[j], parts, "|" );
    if ( num > 0 ){
      classParts[j] = parts;
      if ( num > largest_anal )
	largest_anal = num;
    }
    else {
      // only one, create a dummy
      vector<string> dummy;
      dummy.push_back( classes[j] );
      classParts[j] = dummy;
    }
  }
  //
  // now expand the result
  vector<vector<string> > result;
  for ( int step=0; step < largest_anal; ++step ){
    vector<string> item(classParts.size());
    for ( size_t k=0; k < classParts.size(); ++k ) {
      item[k] = find_class( step, classParts[k], largest_anal );
    }
    result.push_back( item );
  }
  return result;
}
#else

bool next_perm( vector< vector<string>::const_iterator >& its,
		const vector<vector<string> >& parts ){
  for ( size_t i=0; i < parts.size(); ++i ){
    ++its[i];
    if ( its[i] == parts[i].end() ){
      if ( i == parts.size() -1 )
	return false;
      its[i] = parts[i].begin();
    }
    else
      return true;
  }
  return false;
}

vector<vector<string> > generate_all_perms( const vector<string>& classes ){

  // determine all alternative analyses
  // store every part in a vector of string vectors
  vector<vector<string> > classParts;
  classParts.resize( classes.size() );
  for ( unsigned int j=0; j< classes.size(); ++j ){
    vector<string> parts;
    int num = split_at( classes[j], parts, "|" );
    if ( num > 0 ){
      classParts[j] = parts;
    }
    else {
      // only one, create a dummy
      vector<string> dummy;
      dummy.push_back( classes[j] );
      classParts[j] = dummy;
    }
  }
  //
  // now expand
  vector< vector<string>::const_iterator > its( classParts.size() );
  for ( size_t i=0; i<classParts.size(); ++i ){
    its[i] = classParts[i].begin();
  }
  vector<vector<string> > result;
  bool more = true;
  while ( more ){
    vector<string> item(classParts.size());
    for( size_t j=0; j< classParts.size(); ++j ){
      item[j] = *its[j];
    }
    result.push_back( item );
    more = next_perm( its, classParts );
  }
  return result;
}
#endif

void Mbma::clearAnalysis(){
  for ( size_t i=0; i < analysis.size(); ++i ){
    delete analysis[i];
  }
  analysis.clear();
}

void Mbma::execute( const UnicodeString& word,
		    const vector<string>& classes ){
  clearAnalysis();
  vector<vector<string> > allParts = generate_all_perms( classes );
  if ( debugFlag ){
    string out = "alternatives: word=" + UnicodeToUTF8(word) + ", classes=<";
    for ( size_t i=0; i < classes.size(); ++i )
      out += classes[i] + ",";
    out += ">";
    *Log(mbmaLog) << out << endl;
    *Log(mbmaLog) << "allParts : " << allParts << endl;
  }

  // now loop over all the analysis
  for ( unsigned int step=0; step < allParts.size(); ++step ) {
    Rule rule( allParts[step], word, mbmaLog, debugFlag );
    if ( rule.performEdits() ){
      rule.reduceZeroNodes();
      if ( debugFlag ){
	*Log(mbmaLog) << "after reduction: " << rule << endl;
      }
      rule.resolve_inflections();
      if ( debugFlag ){
	*Log(mbmaLog) << "after resolving: " << rule << endl;
      }
      MBMAana *tmp = new MBMAana( rule, doDaring );
      if ( debugFlag ){
	*Log(mbmaLog) << "1 added Inflection: " << tmp << endl;
      }
      analysis.push_back( tmp );
    }
    else if ( debugFlag ){
      *Log(mbmaLog) << "rejected rule: " << rule << endl;
    }
  }
}

void Mbma::addMorph( Word *word,
		     const vector<string>& morphs ) const {
  KWargs args;
  args["set"] = mbma_tagset;
  MorphologyLayer *ml;
#pragma omp critical(foliaupdate)
  {
    try {
      ml = word->addMorphologyLayer( args );
    }
    catch( const exception& e ){
      *Log(mbmaLog) << e.what() << " addMorph failed." << endl;
      exit(EXIT_FAILURE);
    }
  }
  addMorph( ml, morphs );
}

void Mbma::addBracketMorph( Word *word,
			    const string& wrd,
			    const string& tag ) const {
  //  *Log(mbmaLog) << "addBracketMorph(" << wrd << "," << tag << ")" << endl;
  KWargs args;
  args["set"] = mbma_tagset;
  MorphologyLayer *ml;
#pragma omp critical(foliaupdate)
  {
    try {
      ml = word->addMorphologyLayer( args );
    }
    catch( const exception& e ){
      *Log(mbmaLog) << e.what() << " addBracketMorph failed." << endl;
      exit(EXIT_FAILURE);
    }
  }
  args["class"] = "stem";
  Morpheme *result = new Morpheme( word->doc(), args );
  args.clear();
  args["value"] = wrd;
  args["offset"] = "0";
  TextContent *t = new TextContent( args );
#pragma omp critical(foliaupdate)
  {
    result->append( t );
  }
  args.clear();
  args["subset"] = "structure";
  args["class"]  = "[" + wrd + "]";
#pragma omp critical(foliaupdate)
  {
    folia::Feature *feat = new folia::Feature( args );
    result->append( feat );
  }
  args.clear();
  args["set"] = clex_tagset;
  args["cls"] = tag;
#pragma omp critical(foliaupdate)
  {
    result->addPosAnnotation( args );
  }
#pragma omp critical(foliaupdate)
  {
    ml->append( result );
  }
}

void Mbma::addBracketMorph( Word *word,
			    const BracketNest *brackets ) const {
  KWargs args;
  args["set"] = mbma_tagset;
  MorphologyLayer *ml;
#pragma omp critical(foliaupdate)
  {
    try {
      ml = word->addMorphologyLayer( args );
    }
    catch( const exception& e ){
      *Log(mbmaLog) << e.what() << " addBracketMorph failed." << endl;
      exit(EXIT_FAILURE);
    }
  }
  Morpheme *m = brackets->createMorpheme( word->doc(),
					  mbma_tagset,
					  clex_tagset );
  if ( m ){
#pragma omp critical(foliaupdate)
    {
      ml->append( m );
    }
  }
}

void Mbma::addMorph( MorphologyLayer *ml,
		     const vector<string>& morphs ) const {
  int offset = 0;
  for ( size_t p=0; p < morphs.size(); ++p ){
    KWargs args;
    args["set"] = mbma_tagset;
    Morpheme *m = new Morpheme( ml->doc(), args );
#pragma omp critical(foliaupdate)
    {
      ml->append( m );
    }
    args.clear();
    args["value"] = morphs[p];
    args["offset"] = toString(offset);
    TextContent *t = new TextContent( args );
    offset += morphs[p].length();
#pragma omp critical(foliaupdate)
    {
      m->append( t );
    }
  }
}

bool mbmacmp( MBMAana *m1, MBMAana *m2 ){
  return m1->getKey(false).length() > m2->getKey(false).length();
}

void Mbma::filterTag( const string& head,  const vector<string>& feats ){
  // first we select only the matching heads
  if (debugFlag){
    *Log(mbmaLog) << "filter with tag, head: " << head
		  << " feats: " << feats << endl;
    *Log(mbmaLog) << "filter:start, analysis is:" << endl;
    int i=1;
    for(vector<MBMAana*>::const_iterator it=analysis.begin(); it != analysis.end(); it++)
      *Log(mbmaLog) << i++ << " - " << *it << endl;
  }
  map<string,string>::const_iterator tagIt = TAGconv.find( head );
  if ( tagIt == TAGconv.end() ) {
    // this should never happen
    throw ValueError( "unknown head feature '" + head + "'" );
  }
  if (debugFlag){
    *Log(mbmaLog) << "#matches: " << tagIt->first << " " << tagIt->second << endl;
  }
  vector<MBMAana*>::iterator ait = analysis.begin();
  while ( ait != analysis.end() ){
    string tagI = (*ait)->getTag();
    if ( tagIt->second == tagI || ( tagIt->second == "N" && tagI == "PN" ) ){
      if (debugFlag)
	*Log(mbmaLog) << "comparing " << tagIt->second << " with "
		      << tagI << " (OK)" << endl;
      ait++;
    }
    else {
      if (debugFlag)
	*Log(mbmaLog) << "comparing " << tagIt->second << " with "
		      << tagI << " (rejected)" << endl;
      delete *ait;
      ait = analysis.erase( ait );
    }
  }
  if (debugFlag){
    *Log(mbmaLog) << "filter: analysis after first step" << endl;
    int i=1;
    for(vector<MBMAana*>::const_iterator it=analysis.begin(); it != analysis.end(); it++)
      *Log(mbmaLog) << i++ << " - " << *it << endl;
  }

  if ( analysis.size() < 1 ){
    if (debugFlag ){
      *Log(mbmaLog) << "analysis is empty so skip next filter" << endl;
    }
    return;
  }
  // ok there are several analysis left.
  // try to select on the features:
  //
  // find best match
  // loop through all subfeatures of the tag
  // and match with inflections from each m
  set<MBMAana *> bestMatches;
  int max_count = 0;
  for ( size_t q=0; q < analysis.size(); ++q ) {
    int match_count = 0;
    string inflection = analysis[q]->getInflection();
    if (debugFlag){
      *Log(mbmaLog) << "matching " << inflection << " with " << feats << endl;
    }
    for ( size_t u=0; u < feats.size(); ++u ) {
      map<string,string>::const_iterator conv_tag_p = TAGconv.find(feats[u]);
      if (conv_tag_p != TAGconv.end()) {
	string c = conv_tag_p->second;
	if (debugFlag){
	  *Log(mbmaLog) << "found " << feats[u] << " ==> " << c << endl;
	}
	if ( inflection.find( c ) != string::npos ){
	  if (debugFlag){
	    *Log(mbmaLog) << "it is in the inflection " << endl;
	  }
	  match_count++;
	}
      }
    }
    if (debugFlag)
      *Log(mbmaLog) << "score: " << match_count << " max was " << max_count << endl;
    if (match_count >= max_count) {
      if (match_count > max_count) {
	max_count = match_count;
	bestMatches.clear();
      }
      bestMatches.insert(analysis[q]);
    }
  }
  // so now we have "the" best matches.
  // Weed the rest
  ait = analysis.begin();
  while ( ait != analysis.end() ){
    if ( bestMatches.find( *ait ) != bestMatches.end() ){
      ++ait;
    }
    else {
      delete *ait;
      ait = analysis.erase( ait );
    }
  }
  if (debugFlag){
    *Log(mbmaLog) << "filter: analysis after second step" << endl;
    int i=1;
    for( vector<MBMAana*>::const_iterator it=analysis.begin();
	 it != analysis.end();
	 ++it )
      *Log(mbmaLog) << i++ << " - " << *it << endl;
  }
  //
  // but now we still might have doubles
  //
  map<UnicodeString, MBMAana*> unique;
  ait=analysis.begin();
  while ( ait != analysis.end() ){
    UnicodeString tmp = (*ait)->getKey( doDaring );
    unique[tmp] = *ait;
    ++ait;
  }
  // so we have map of 'equal' analysis
  set<MBMAana*> uniqueAna;
  map<UnicodeString, MBMAana*>::const_iterator uit=unique.begin();
  while ( uit != unique.end() ){
    uniqueAna.insert( uit->second );
    ++uit;
  }
  // and now a set of all MBMAana's that are really different.
  // remove all analysis that aren't in that set.
  ait=analysis.begin();
  while ( ait != analysis.end() ){
    if ( uniqueAna.find( *ait ) != uniqueAna.end() ){
      ++ait;
    }
    else {
      delete *ait;
      ait = analysis.erase( ait );
    }
  }
  if ( debugFlag ){
    *Log(mbmaLog) << "filter: analysis before sort on length:" << endl;
    int i=1;
    for(vector<MBMAana*>::const_iterator it=analysis.begin(); it != analysis.end(); it++)
      *Log(mbmaLog) << i++ << " - " << *it << " " << (*it)->getKey(false)
		    << " (" << (*it)->getKey(false).length() << ")" << endl;
    *Log(mbmaLog) << "" << endl;
  }
  // Now we have a small list of unique and differtent analysis.
  // We assume the 'longest' analysis to be the best.
  // So we prefer '[ge][maak][t]' over '[gemaak][t]'
  // Therefor we sort on (unicode) string length
  sort( analysis.begin(), analysis.end(), mbmacmp );

  if ( debugFlag){
    *Log(mbmaLog) << "filter: definitive analysis:" << endl;
    int i=1;
    for(vector<MBMAana*>::const_iterator it=analysis.begin(); it != analysis.end(); it++)
      *Log(mbmaLog) << i++ << " - " << *it << endl;
    *Log(mbmaLog) << "done filtering" << endl;
  }
  return;
}

void Mbma::getFoLiAResult( Word *fword, const UnicodeString& uword ) const {
  if ( analysis.size() == 0 ){
    // fallback option: use the word and pretend it's a morpheme ;-)
    if ( debugFlag ){
      *Log(mbmaLog) << "no matches found, use the word instead: "
		    << uword << endl;
    }
    if ( doDaring ){
      addBracketMorph( fword, UnicodeToUTF8(uword), "UNK" );
    }
    else {
      vector<string> tmp;
      tmp.push_back( UnicodeToUTF8(uword) );
      addMorph( fword, tmp );
    }
  }
  else {
    vector<MBMAana*>::const_iterator sit = analysis.begin();
    while( sit != analysis.end() ){
      if ( doDaring ){
	addBracketMorph( fword, (*sit)->getBrackets() );
      }
      else {
	addMorph( fword, (*sit)->getMorph() );
      }
      ++sit;
    }
  }
}

void Mbma::addDeclaration( Document& doc ) const {
  doc.declare( AnnotationType::MORPHOLOGICAL, mbma_tagset,
	       "annotator='frog-mbma-" +  version +
	       + "', annotatortype='auto', datetime='" + getTime() + "'");
  if ( doDaring ){
    doc.declare( AnnotationType::POS, clex_tagset,
		 "annotator='frog-mbma-" +  version +
		 + "', annotatortype='auto', datetime='" + getTime() + "'");
  }
}

UnicodeString Mbma::filterDiacritics( const UnicodeString& in ) const {
  if ( transliterator ){
    UnicodeString result = in;
    transliterator->transliterate( result );
    return result;
  }
  else
    return in;
}

void Mbma::Classify( Word* sword ){
  if ( sword->isinstance(PlaceHolder_t) )
    return;
  UnicodeString uWord;
  PosAnnotation *pos;
  string head;
  string token_class;
#pragma omp critical(foliaupdate)
  {
    uWord = sword->text();
    pos = sword->annotation<PosAnnotation>( cgn_tagset );
    head = pos->feat("head");
    token_class = sword->cls();
  }
  if (debugFlag)
    *Log(mbmaLog) << "Classify " << uWord << "(" << pos << ") ["
		  << token_class << "]" << endl;
  if ( filter )
    uWord = filter->filter( uWord );
  if ( head == "LET" || head == "SPEC" ){
    // take over the letter/word 'as-is'.
    string word = UnicodeToUTF8( uWord );
    if ( doDaring ){
      addBracketMorph( sword, word, head );
    }
    else {
      vector<string> tmp;
      tmp.push_back( word );
      addMorph( sword, tmp );
    }
  }
  else {
    UnicodeString lWord = uWord;
    if ( head != "SPEC" )
      lWord.toLower();
    Classify( lWord );
    vector<string> featVals;
#pragma omp critical(foliaupdate)
    {
      vector<folia::Feature*> feats = pos->select<folia::Feature>();
      for ( size_t i = 0; i < feats.size(); ++i )
	featVals.push_back( feats[i]->cls() );
    }
    filterTag( head, featVals );
    getFoLiAResult( sword, lWord );
  }
}

void Mbma::Classify( const UnicodeString& word ){
  UnicodeString uWord = filterDiacritics( word );
  vector<string> insts = make_instances( uWord );
  vector<string> classes;
  for( size_t i=0; i < insts.size(); ++i ) {
    string ans;
    MTree->Classify( insts[i], ans );
    if ( debugFlag ){
      *Log(mbmaLog) << "itt #" << i << " " << insts[i] << " ==> " << ans
		    << ", depth=" << MTree->matchDepth() << endl;
    }
    classes.push_back( ans);
  }

  // fix for 1st char class ==0
  if ( classes[0] == "0" )
    classes[0] = "X";
  execute( uWord, classes );
}

vector<vector<string> > Mbma::getResult() const {
  vector<vector<string> > result;
  for (vector<MBMAana*>::const_iterator it=analysis.begin();
       it != analysis.end();
       it++ ){
    if ( doDaring ){
      stringstream ss;
      ss << (*it)->getBrackets()->put( true ) << endl;
      vector<string> mors;
      mors.push_back( ss.str() );
      result.push_back( mors );
    }
    else {
      vector<string> mors = (*it)->getMorph();
      if ( debugFlag ){
	*Log(mbmaLog) << "Morphs " << mors << endl;
      }
      result.push_back( mors );
    }
  }
  if ( debugFlag ){
    *Log(mbmaLog) << "result of morph analyses: " << result << endl;
  }
  return result;
}

ostream& operator<< ( ostream& os, const MBMAana& a ){
  os << "tag: " << a.getTag() << " infl:" << a.getInflection() << " morhemes: "
     << a.rule.extract_morphemes() << " description: " << a.getDescription();
  return os;
}

ostream& operator<< ( ostream& os, const MBMAana *a ){
  if ( a )
    os << *a;
  else
    os << "no-mbma";
  return os;
}
