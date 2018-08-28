/* ex: set tabstop=8 expandtab: */
/*
  Copyright (c) 2006 - 2018
  CLST  - Radboud University
  ILK   - Tilburg University

  This file is part of frog:

  A Tagger-Lemmatizer-Morphological-Analyzer-Dependency-Parser for
  several languages

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
      https://github.com/LanguageMachines/frog/issues
  or send mail to:
      lamasoftware (at ) science.ru.nl

*/

#ifndef CGN_TAGGER_MOD_H
#define CGN_TAGGER_MOD_H

#include "frog/FrogData.h"
#include "frog/tagger_base.h"

class CGNTagger: public BaseTagger {
 public:
  explicit CGNTagger( TiCC::LogStream *l ): BaseTagger( l, "tagger" ){};
  bool init( const TiCC::Configuration& );
  void addDeclaration( folia::Document& ) const;
  void addDeclaration( folia::Processor& ) const;
  void post_process( const std::vector<folia::Word*>& );
  void post_process( frog_data& );
  std::vector<folia::Word*> add_result( folia::Sentence*,
					const frog_data& ) const;
  std::string getSubSet( const std::string& ,
			 const std::string&,
			 const std::string& ) const;
  void set_text_redundancy( const std::string& tr ){ textredundancy = tr; };
 private:
  void addTag( frog_record&, const std::string&, double );
  void addTag( folia::Word *, const std::string&, double );
  void fillSubSetTable();
  bool fillSubSetTable( const std::string&, const std::string& );
  std::string textredundancy;
  std::multimap<std::string,std::string> cgnSubSets;
  std::multimap<std::string,std::string> cgnConstraints;
};

#endif // CGN_TAGGER_MOD_H
