/*
  $Id$
  $URL$

  Copyright (c) 2006 - 2011
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
                                    
#ifndef __MDMA_MOD__
#define __MBMA_MOD__

class MBMAana;

struct waStruct {
  UnicodeString word;
  std::string act;
  void clear(){
    word = "";
    act = "";
  }
};

class Configuration;

class Mbma {
 public:
  Mbma();
  ~Mbma() { cleanUp(); };
  bool init( const Configuration& );
  void Classify( const UnicodeString& );
  std::string postprocess( const UnicodeString& word,
			   const std::string& inputTag );
 private:
  void cleanUp();
  bool readsettings( const std::string&, const std::string& );
  void fillMaps();
  void init_cgn( const std::string& );
  std::vector<std::string> make_instances( const UnicodeString& word );
  std::string calculate_ins_del( const std::string& in_class, 
				 std::string& deletestring,
				 std::string& insertstring );
  std::vector<waStruct> Step1( unsigned int step, 
			       const UnicodeString& word, int nranal,
			       const std::vector<std::vector<std::string> >& classParts,
			       const std::string& basictags );
  void resolve_inflections( std::vector<waStruct>& , const std::string& );
  MBMAana addInflect( const std::vector<waStruct>& ana,
		      const std::string&, const std::string&  );
  MBMAana inflectAndAffix( const std::vector<waStruct>& ana );
  void execute( const UnicodeString& word, 
		const std::vector<std::string>& classes );
  int debugFlag;
  std::string MTreeFilename;
  Timbl::TimblAPI *MTree;
  std::map<char,std::string> iNames;  
  std::map<std::string,std::string> tagNames;  
  std::map<std::string,std::string> TAGconv;
  std::vector<MBMAana> analysis;
};

class MBMAana {
  friend std::ostream& operator<< ( std::ostream& , const MBMAana& );
  public:
  MBMAana() {
    tag = "";
    infl = "";
    morphemes = "";
    description = "";    
  };
 MBMAana( const std::string& _tag,
	  const std::string& _infl,
	  const std::string& _mo, 
	  const std::string& _des ): 
  tag(_tag),infl(_infl),morphemes(_mo), description(_des) {};
  
  ~MBMAana() {};
  
  std::string getTag() const {
    return tag;
  };
  
  std::string getInflection() const {
    return infl;
  };
  
  std::string getMorph() const {
    return morphemes;
  };
  
 private:
  std::string tag;
  std::string infl;
  std::string morphemes;
  std::string description;  
};

#endif