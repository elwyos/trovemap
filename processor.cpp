//////////////////////////////////////////////////////////////////////////////////////////
#include "processor.hpp"
#include "../shing/shinglooper.hpp"
#include "../shing/oust.hpp"
#include "../shing/morestring.hpp"
#include "../shing/once.hpp"
#include "../shing/context.hpp"
#include "../shing/schfile.hpp"
//////////////////////////////////////////////////////////////////////////////////////////
#include "../shing/asynca.hpp"
#include "../shing/ripcurl.hpp"
#include "../shing/xmlparser.hpp"
#include "../shing/textfile.hpp"
#include "../shing/crossplatform.hpp"
//////////////////////////////////////////////////////////////////////////////////////////
#include "../nlp/commonwords.hpp"
//////////////////////////////////////////////////////////////////////////////////////////
#include "../skynet/skynet.hpp"
#include "../shing/consolearg.hpp"
#include <algorithm>
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
namespace Gov{
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
YearMonth::YearMonth(){
   year = 0;
   month = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool YearMonth::operator<(const YearMonth& rhs) const{
   if(year < rhs.year)
      return true;
   else if (year > rhs.year)
      return false;
   
   return month < rhs.month;
}
//////////////////////////////////////////////////////////////////////////////////////////
int YearMonth::monthdiff(const YearMonth& thebase) const{
   return (year - thebase.year) * 12 + (month - thebase.month);
}
//////////////////////////////////////////////////////////////////////////////////////////
YearMonth YearMonth::monthNeigh(int thediff){
   YearMonth ret = *this;
   ret.month += thediff;
   while(ret.month > 12){
      ret.month -= 12;
      ret.year++;
   }
   
   while(ret.month < 1){
      ret.month += 12;
      ret.year--;
   }
   return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& os, const YearMonth& ym){
   os << ym.year << "/" << ym.month;
   return os;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Article::Article(){
   pagesequence = 5;
   monthday = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Article::receiveFull(const string& thefull){
   // it's escaped HTML..
   
   Shing::XMLParser parsa(thefull);
   
   articletext = "";

   parsa.onTag = [&, this](Shing::XMLTag& tag){
      
      if(tag == "p" && tag.close){
         
         articletext.append(2, '\n');
         
      } else if (tag == "span" && tag.close){
         articletext.append(1, '\n');
      }
      
   };
   
   parsa.onContent = [&, this](const string& thecontent){
      articletext.append(thecontent); // yeah
   };
   
   parsa.parse();
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Article::receiveDateString(const string& thedatestring){
   EasyParser parsa(thedatestring);
   ym.year = parsa.parseNumber();
   parsa << '-';
   ym.month = parsa.parseNumber();
   parsa << '-';
   monthday = parsa.parseNumber();
   // cool
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConnection::PhraseConnection(){
   strength = 0;
   tempstrength = 0;
   tempstrength = 0;
   origin = NULL;
   target = NULL;
   topNcount = 0;
   
   needexport = false;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
float& PhraseConnection::strengthForMonth(const YearMonth& themonth){
   return strengthForBucket(processor().bucketForMonth(themonth));
}
//////////////////////////////////////////////////////////////////////////////////////////
float & PhraseConnection::strengthForBucket(int thebucket){
   while(thebucket >= strengths.size())
      strengths.push_back(0);
   
   return strengths[thebucket];
}
//////////////////////////////////////////////////////////////////////////////////////////
void PhraseConnection::summarizeStrength(){
   
   float scoretotal = 0;
   
   // .. based on strengths..
   int zeroes = 0;
   int i;
   int maxbucket = Gov::processor().maxbucket;
   for(i = 0; i < maxbucket + 1; i++){
      if(i < strengths.size()){
         if(strengths[i] < 0.2f){
            zeroes++;
         }
         scoretotal += strengths[i];
      } else {
         zeroes++;
      }
   }
   
   float scoreavg = scoretotal / (maxbucket + 1);
   
   float zeroratio = (zeroes / (float)(maxbucket + 1));
   
   summarizedstrength = scoreavg * pow(zeroratio, 2.0f);
   summarizedstrength *= origin->summarizedstrength * target->summarizedstrength;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Phrase::Phrase(){
   inited = false;
   isnew = true;
   originalsearchfrequency = 0;
   strength = 0;
   summarizedstrength = 0;
   isname = false;
   iscountry = false;
   issimplyname = false;
   
   needexport = false;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Phrase::summarizeStrength(){
   
   float mult1 = max(signedpow(originalsearchfrequency, 0.7f), 1.0f);
   
   float lemax = 0;
   sloopvec(strengths, moo)
      lemax = max(lemax, moo); // yeah, simple here.
   sloopend
   
   summarizedstrength = mult1 * pow(lemax, 0.4f); // yeah
   
   if(isname && !iscountry){
      summarizedstrength *= 0.15f;
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Phrase::init(const string& thephrase){
   phrase = thephrase;
   inited = false;
   
   iscountry = NLP::wordIsCountry(thephrase, true);
   isname = NLP::wordIsName(thephrase);
   
   issimplyname = isname && !iscountry;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConnection& Phrase::connectionTo(Phrase * rhs){
   
   if(rhs < this){
      return rhs->connectionTo(this); // yep
   }
   
   auto it = phraseconnections.find(rhs);
   if(it == phraseconnections.end()){
      PhraseConnection& ret = phraseconnections[rhs];
      
      ret.origin = this;
      ret.target = rhs;
      
      processor().allconns.insert(&ret);
      
      conns.push_back(&ret);
      rhs->conns.push_back(&ret);
      
      return ret;
   }
   
   return it->second;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool topn_comparer(PhraseConnection * one, PhraseConnection * two){
   return one->summarizedstrength > two->summarizedstrength;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Phrase::calculateTopNConns(int thetopn){
   sort(conns.begin(), conns.end(), topn_comparer);
   
   int counta = 0;
   
   sloopvec(conns, moo)
   
      moo->topNcount++;
   
      counta++;
      if(counta >= thetopn)
         break;
   
   sloopend
   
}
//////////////////////////////////////////////////////////////////////////////////////////
float& Phrase::strengthForMonth(const YearMonth& themonth){
   return strengthForBucket(processor().bucketForMonth(themonth));
}
//////////////////////////////////////////////////////////////////////////////////////////
float & Phrase::strengthForBucket(int thebucket){
   while(thebucket >= strengths.size())
      strengths.push_back(0);
   
   return strengths[thebucket];
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Processor::Processor():
   requestcache("reqcache"){
   
   skyn.connect("localhost", 1440);
   
   maxbucket = 11;

}
//////////////////////////////////////////////////////////////////////////////////////////
Phrase& Processor::obtainPhrase(const string& thephrase){
   
   if(phrasetrie.count(thephrase) == 0){
      Phrase& ref = phrases[thephrase];
      phrasetrie[thephrase] = &ref;
      ref.init(thephrase);
      return ref;
   } else {
      return *phrasetrie[thephrase];   
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConnection& Processor::obtainConnection(Phrase& one, Phrase& two){
   
   return obtainConnection(&one, &two);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConnection& Processor::obtainConnection(Phrase * one, Phrase * two){
   
   assert(one != two);
   
   if(two < one)
      return obtainConnection(two, one);
   
   return one->connectionTo(two);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Processor::loadWikiTitles(){

   int counta = 0;
   
   ofstream theoust("titles-filtered.txt");

   textfileloop("/home/bennylava/temp2/wikipedia-titles.txt")
      
      if(theline.size() == 0){
      } else {
         
         if(isalpha(theline[0])){
            
            string whatever = theline;
            int i;
            for(i = 0; i < whatever.size(); i++){
               if(whatever[i] == '_')
                  whatever[i] = ' ';
            }
            
            string processed;
            
            EasyParser parsa(whatever);
            
            if(parsa << "List"){
            } else {
            
               while(parsa.hasChar()){
                  if(parsa--.peekingAtLabelStart()){
                     string moo = parsa--.parseLabel();
                     if(processed.size() != 0)
                        processed.append(1, ' ');
                     processed.append(moo);
                  } else {
                     break;
                  }
               }
               
               if(processed.size() > 3 && processed.size() < 24){
               
                  string lowered = __toLower(processed);
               
                  Phrase& phrase = obtainPhrase(lowered);
                  if(phrase.isnew){
                     theoust << lowered << endl;
                     phrase.isnew = false;
                  }
                  
                  counta++;
                  if(counta % 1000 == 0)
                     cout << "check: " << counta << " : " << lowered << endl;
                     
               }
            }
         }
         
      }
      
   textfileloopend
   
   cout << "Finally.. total : " << phrases.size() << endl;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
bool ignoreThisWord(const string& mehed){
   
   int countalpha = 0;
   sloopvec(mehed, eh)
      if(isalpha(eh))
         countalpha++;
   sloopend
   
   float alpharatio = countalpha / max((float)mehed.size(), 1.0f);
   
   if(mehed.size() <= 3)
      return true;
      
   if(alpharatio < 0.7f)
      return true;
   
   if(NLP::wordIsCommon(mehed))
      return true;
      
   if(NLP::wordIsWeekdayOrMonth(mehed))
      return true;
   
   if(Morestring::beginsWith(mehed, "the "))
      return true;
   
   return false;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Processor::loadQueenslandSearches(){

   ofstream theoust("titles-filtered.txt");

   int counta = 0;

   textfileloop("/home/bennylava/temp2/queenslandlibrary_totalsearches.txt")
   
      vector<string> splitted;
      __splitOn(theline, '|', splitted);
      
      if(splitted.size() == 3){
         // bingo
         string phrasebit = Morestring::deencaseLiteral(splitted[0]);
         
         // split by slash
         vector<string> slashed;
         __splitOn(phrasebit, '/', slashed);
         
         int thefreq = __toInt(splitted[1]);
         if(thefreq > 90){
            // anomaly, like the "james", like wtf
         } else {
            sloopvec(slashed, moo)
            
               string mehed = __chompends(__toLower(moo));
               
               if(!ignoreThisWord(mehed)){
            
                  Phrase& phrase = obtainPhrase(mehed);
                  
                  if(phrase.isnew){
                     phrase.isnew = false;
                     theoust << mehed << endl;
                  }
                  
                  phrase.originalsearchfrequency += thefreq;
                  
               }
               
            sloopend
         }
         
      }
      
      if(counta++ % 5000 == 0){
         cout << counta << endl;
      }
      
   textfileloopend
   
   cout << "Finally.. total : " << phrases.size() << endl;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
bool shouldParseArticle(const string& header){
   string lowered = __toLower(header);
   
   if(lowered == "advertising")
      return false;
      
   if(_mos::contains(lowered, "advertisement index"))
      return false;
      
   if(_mos::contains(lowered, "casualty list"))
      return false;
   
   return true;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
ArticleProcessing::ArticleProcessing(Article& theart):
   art(theart){
   // beh
}
//////////////////////////////////////////////////////////////////////////////////////////
void ArticleProcessing::addInstance(Phrase * thephrase, int thelinenum, int thecharnum){
   
   PhraseInstance topush;
   topush.phrase = thephrase;
   topush.linenum = thelinenum;
   topush.charnum = thecharnum;
   instances.push_back(topush);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void ArticleProcessing::calculatePhraseGraphing(){
   
   int maxchar = 0;
   
   if(instances.size() > 50){
      // meh to this, seriously. probably funeral notices
      return;
   }
   
   sloopvec(instances, moo)
      
      sloopvec(instances, mee)
      
         if(&moo != &mee && moo.phrase != mee.phrase){
            
            float strengthmult;
            if(moo.charnum == mee.charnum){
               strengthmult = 1;
            } else {
               strengthmult = 1 / fabs(moo.charnum - mee.charnum) + 1;
            }
            
            strengthmult *= pow(max(1.0f, (float)moo.phrase->originalsearchfrequency), 0.5f);
            strengthmult *= pow(max(1.0f, (float)mee.phrase->originalsearchfrequency), 0.5f);
            
            if(moo.phrase->issimplyname && mee.phrase->issimplyname){
               strengthmult *= 0.2f;
            } else if (moo.phrase->issimplyname || mee.phrase->issimplyname){
               strengthmult *= 0.37f;
            }
            
            PhraseConnection& conn = moo.phrase->connectionTo(mee.phrase);
            conn.tempstrength += strengthmult;
            
            connections.insert(&conn);
            
         }
         
      sloopend
      
      maxchar = max(moo.charnum, maxchar);
      
   sloopend
   
   maxchar = max(1, maxchar);
   
   
   if(!gplaice.has("silence")){
      oustoust.push(3);
   }
   
   sloopset(connections, moo)
      
      float lescore = moo->tempstrength / (float)maxchar;
      
      if(lescore> 0.1f){

         if(!gplaice.has("silence")){
            oustoust[240] << moo->origin->phrase << " -- " << lescore << " -> " << moo->target->phrase << " // " << maxchar << endl;
         }
         
         //Skynet::Edge * edge = processor().skyn.undirectedEdge(moo->origin->phrase, "conn", moo->target->phrase);
         // yeah..
         moo->strength += 1.0f;
         moo->strengthForMonth(art.ym) += 1.0f; // yep
      }
   
      // reset it
      moo->tempstrength = 0;
   sloopend
   

   if(!gplaice.has("silence")){
      oustoust.pop();
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Processor::processArticle(Article& article){
   
   if(!shouldParseArticle(article.heading))
      return;
   
   // here goes nothing, yeah
   // raw text
   
   EasyParser parsa(article.articletext);
   
   ArticleProcessing processing(article);
   
   if(!gplaice.has("silence")){
      oustoust[220] << "Processing : " >> 33 << article.heading 
         >> 240 << " @" >> 250 << article.ym.year << "/" << article.ym.month << "/" << article.monthday
         << ozendl;
      oustoust.push(3);
      
      oustoust[245];
      
   }
   
   bool gotany = false;
   
   int linenum = 0;
   int charnum = 0;
   
   while(parsa--.hasChar()){
      ptriepath<Phrase*> furthest = parsa.parseTriePathCaseless(phrasetrie);
      
      if(furthest.isvalued()){
      
         if(!gplaice.has("silence")){
            if(gotany)
               oustoust << ", ";
         
            oustoust << furthest.pathsofar();
         }
         
         processing.addInstance(furthest.getval(), linenum, charnum);
         
         // just add teh phrase for now
         
         float theadd = 1.0f;
         if(article.pagesequence == 1){
            theadd = 5.0f;
            // can't assume anything out of the rest..
         }
         // if article page is 1..
         
         furthest.getval()->strengthForMonth(article.ym) += theadd; // yeah
         furthest.getval()->strength += theadd;
         
         gotany = true;
      }
      
      string thelabel = parsa--.parseLabel();
      if(thelabel == ""){
         if(parsa == '\n'){
            linenum++;
         }
         parsa.accept(); // accept anything
         charnum++;
      } else {
         charnum += thelabel.size();
      }
      
   }
   
   if(!gplaice.has("silence")){
      if(gotany)
         oustoust << ozendl;
      
      
      oustoust.pop();
   }
   
   processing.calculatePhraseGraphing();
   
}
//////////////////////////////////////////////////////////////////////////////////////////
int Processor::bucketForMonth(const YearMonth& ym){
   int ret = ym.monthdiff(querystart);
   maxbucket = max(ret, maxbucket);
   return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Processor::main(int theyear){
   
   querystart.year = theyear;
   querystart.month = 1;
   
   int i;
   
   int maxnum = (120000 / 100);
   
   for(i = 0; i <= maxnum; i++){
      bool succ = processTrove(querystart.year, i * 100);
      if(!succ){
         oustoust[196] << "Errored during : " >> 245 << (i + 1) << " / " << maxnum << ozendl;
         break; // beh
      } else {
         oustoust[33] << "Done : " >> 245 << (i + 1) << " / " << maxnum << ozendl;
      }
   }
   
   // and then see the graph, I suppose
   
   // export it for this period, I suppose
   
   cout << "Done! exporting.." << endl;
   
   string filename = "graph_" + __toString(querystart.year) + ".out";
   
   ofstream out(filename.c_str());
   
   
   out << "--phrases--" << endl;
   
   sloopmapex(phrases, eh, moo)
      
      if(moo.strength > 3.5f){
      
         out << eh << "/";
      
         bool gotany = false;
         
         sloopvec(moo.strengths, eh)
            if(gotany)
               out << ",";
            out << eh;
            gotany = true;
         sloopend
         
         out << endl;
         
      }
      
   sloopend
   
   out << "--connections--" << endl;
   
   sloopset(allconns, moo)
      if(moo->strength > 2.5f){
         
         out << moo->origin->phrase << "/" << moo->target->phrase << "/";
         
         bool gotany = false;
         
         sloopvec(moo->strengths, eh)
            if(gotany)
               out << ",";
            out << eh;
            gotany = true;
         sloopend
         
         out << endl;

      }
   sloopend
   
   

}
//////////////////////////////////////////////////////////////////////////////////////////
bool Processor::processTrove(int yearstart, int startnum){
   
   // read through the CSV's of both
   
   // api key ul2fnd0p08pgvjeb
   
   string thequery = "date:[" + __toString(yearstart) + " TO " + __toString(yearstart) + "]";
   
   string request = "http://api.trove.nla.gov.au/result?key=ul2fnd0p08pgvjeb&zone=newspaper&q=" 
                           + _mos::urlencode(thequery) 
                           +
                             "&l-title=35"
                             "&n=100"
                             "&s=" + __toString(startnum) + 
                             "&include=articletext%2Ccomments%2Ctags&receivel=full";
   
   string retdata;
   Serror serror;
   
   string reqkey = __toString(yearstart) + "_" + __toString(startnum) + ".req";
   
   bool fromcache = false;
   
   if(requestcache.has(reqkey)){
      retdata = requestcache.get(reqkey);
      
      static int counta = 0;
      counta++;
      if(counta % 10 == 0){
         oustoust[220] << "@ " << startnum << ozendl;
         XPF::millisleep(1500);
      }
      
      fromcache = true;
      
   } else {
   
      Ripcurl ripcurl;
      ripcurl.setURL(request);
      ripcurl.accessprocess();
      
      serror = ripcurl.error;
      retdata = ripcurl.retdata;
      
      if(!serror){
         requestcache.set(reqkey, retdata); // yeah
      }
   
   }
   
   splaice(fromcache ? "silence": "meh");
   
   bool errored = false;
   
   bool gotany = false;
   
   if(serror){
      cout << "Errored : " << serror << endl;
      errored = true;
   } else {
      
      Shing::XMLParser parsa(retdata);
      schfile("retxml.xml") <<= retdata;
      int totalcount = 0;
      int curdisp = 0;
      
      parsa.onTag = [&,this](Shing::XMLTag& tag){
      
         if(tag == "records"){
            if(tag.open){
               nextquery = tag["next"];
               totalcount = __toInt(tag["total"]);
               curdisp = __toInt(tag["s"]);
            } else if (tag.close){
               oustoust[33] << "Records : " >> 220 << curdisp << " / " << totalcount << ozendl;
            }
         } else if(tag == "article"){
            
            if(tag.open){
               curart = Article();
               
               tag("onart");
               
               curart.articleid = tag["id"];
            } else if(tag.close){
               
               gotany = true;
               
               if(true || curart.tags.size() != 0 || curart.comments.size() != 0){
                  
                  /*
                  oustoust[33] << curart.heading >> 240 << ozendl;
                  oustoust[245] << curart.newspapertitle >> 112 << " @ " >> 34 << curart.datestring << ozendl;
                  oustoust[245] << "  ID : " >> 250 << curart.articleid << ozendl;
                  oustoust.push(3);
                     oustoust[240] << curart.articletext << ozendl;
                     
                     if(curart.tags.size() != 0 || curart.comments.size() != 0){
                        oustoust.push(3);
                        oustoust[202] << curart.tags << ozendl;
                        sloopvec(curart.comments, moo)
                              oustoust[23] << moo << ozendl;
                        sloopend
                        
                        oustoust.pop();
                     }
                     
                  oustoust.pop();
                  */
                  
                  processArticle(curart);
                  
               }
               
            }
            
         } else if (tag == "snippet"){
            if(tag.close && parsa.ctx("onart")){
               curart.snippet = tag.simplecontents;
            }
         } else if (tag == "tag"){
            if(tag.close && parsa.ctx("onart")){
               curart.tags.push_back(tag.simplecontents);
            }
         } else if (tag == "comment"){
            if(tag.close && parsa.ctx("onart")){
               curart.comments .push_back( tag.simplecontents);
            }
         } else if (tag == "articletext"){
            if(tag.close && parsa.ctx("onart")){
               curart.receiveFull(tag.simplecontents);
            }
         } else if (tag == "pageSequence"){
            if(tag.close && parsa.ctx("onart")){
               curart.pagesequence = __toInt(tag.simplecontents);
            }
         } else if (tag == "title"){
            if(tag.close && parsa.ctx("onart")){
               curart.newspapertitle = (tag.simplecontents);
            }
         } else if (tag == "date"){
            if(tag.close && parsa.ctx("onart")){
               curart.receiveDateString(tag.simplecontents);
            }
         } else if (tag == "heading"){
            if(tag.close && parsa.ctx("onart")){
               curart.heading = tag.simplecontents;
            }
         }
      };
      
      parsa.parse();
      
      if(parsa.error)
         oustoust[196] << "Errored : " << parsa.error << endl;
      
   }
   
   if(!gotany)
      errored = true;
      
   if(errored){
      requestcache.del(reqkey); // erase it on the cache
   }
   
   return !errored;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Processor::combine(vector<string> filenames){
   
   // we'll hardcode year
   
   int yearbegin = 9999; // for now
   int yearend = 0;
   
   vector<string> validfilenames;
   
   sloopvec(filenames, moo)
      EasyParser parsa(moo);
      parsa << "graph_";
      int gotyear = parsa.parseNumber();
      
      if(gotyear > 1800 && gotyear < 2000){
         yearbegin = min(yearbegin, gotyear);
         yearend = max(yearend, gotyear);
         validfilenames.push_back(moo);
         oustoust[33] << "Valid filename : " >> 240 << moo << ozendl;
      } else {
         oustoust[196] << "Goit year : " << gotyear >> 220 << "@ " >> 240 << moo << endl;
      }
      
   sloopend
   
   maxbucket = (yearend + 1 - yearbegin) * 12;
   
   int linecounta = 0;
   int qualtotal = 0;
   
   float linkstrengths = 0;
   int linktotal = 0;
   
   oustoust[112] << "Valid file name size : " >> 240 << validfilenames.size() << endl;
   oustoust[112] << "Year begin : " << yearbegin << endl;
   
   sloopvec(validfilenames, theyearfile)
      
      oustoust[112] << "Processing : " << theyearfile << endl;
   
      int fileyear = 0;
      {
         EasyParser parsa(theyearfile);
         parsa << "graph_";
         fileyear = parsa.parseNumber();
         oustoust[112] << "File year : " >> 240 << fileyear << endl;
      }
      
      int bucketoffset = (fileyear - yearbegin) * 12;
      assert(bucketoffset >= 0 && bucketoffset < 200); // in case anything weird happened
   
      bool onnodes = false;
      bool onedges = false;
      
      textfileloop(theyearfile)
      
         linecounta++;
         if(linecounta % 1000 == 0){
            oustoust[33] << theyearfile >> 220 << " @ " >> 240 << linecounta << ozendl;
         }
      
         if(theline == "--phrases--"){
            onnodes = true;
            onedges = false;
         } else if (theline == "--connections--"){
            onedges = true;
            onnodes = false;
         } if(onnodes){
            // actually
            
            vector<string> elements;
            __splitOn(theline, '/', elements);
            
            if(elements.size() == 2){
               Phrase& thephrase = obtainPhrase(elements[0]);
               
               // cool
               vector<string> scores;
               __splitOn(elements[1], ',', scores);
               
               int i;
               for(i = 0; i < scores.size(); i++){
                  thephrase.strengthForBucket(i + bucketoffset) = __toInt(scores[i]);
               }
               
               thephrase.summarizeStrength();
               
            }
            
         } else if (onedges){
            
            vector<string> elements;
            __splitOn(theline, '/', elements);
            
            if(elements.size() == 3){
               Phrase& phraseone = obtainPhrase(elements[0]);
               Phrase& phrasetwo = obtainPhrase(elements[1]);
               
               PhraseConnection& conn = obtainConnection(phraseone, phrasetwo);
               
               // cool
               vector<string> scores;
               __splitOn(elements[2], ',', scores);
               
               int i;
               for(i = 0; i < scores.size(); i++){
                  conn.strengthForBucket(i + bucketoffset) = __toInt(scores[i]);
                  
               }
               
            }
            
         }
      textfileloopend
   
   sloopend
   
   
   sloopset(allconns, moo)
      moo->summarizeStrength();
      linkstrengths += moo->summarizedstrength;
      linktotal++;
   sloopend
   
   // now, loop through all edges. disable if edge is not on top of the phrases connectivity..
   sloopmapex(phrases, eh, moo)
      moo.calculateTopNConns(40);
   sloopend
   
   sloopset(allconns, moo)
      if(moo->topNcount == 2){
         // good
         if(moo->summarizedstrength > 3){
            Skynet::Edge * edge = processor().skyn.undirectedEdge(moo->origin->phrase, "conn", moo->target->phrase);
            moo->needexport = true;
            moo->origin->needexport = true;
            moo->target->needexport = true;
            qualtotal++;
         }
      }
   sloopend
   
   ofstream fout("finalgraph.out");
   
   fout << "yearbegin:" << yearbegin << endl;
   fout << "---nodes---" << endl;
   
   sloopmapex(phrases, eh, moo)
      
      if(moo.needexport){
         fout << moo.phrase << "|";
         
         if(NLP::wordIsCountry(moo.phrase, true)){
            fout << "c";
         }
         if (NLP::wordIsName(moo.phrase)){
            fout << "n";
         }
         fout << "|";
         
         bool gotany = false;
         
         sloopvec(moo.strengths, str)
            if(gotany)
               fout << ",";
            fout << str;
            gotany = true;
         sloopend
         
         fout << endl;
      }
   sloopend
   
   fout << "---edges---" << endl;
   sloopset(allconns, moo)
      if(moo->needexport){
         fout << moo->origin->phrase << "|" << moo->target->phrase << "|";
         
         bool gotany = false;
         
         sloopvec(moo->strengths, str)
            if(gotany)
               fout << ",";
            fout << str;
            gotany = true;
         sloopend
         
         fout << endl;
      }
   sloopend
   
   
   // we'll test it first
   cout << "Qual total : " << qualtotal << endl;
   cout << "Average : " << (linkstrengths / linktotal) << endl;
   cout << "linktotal : " << linktotal << endl;
   
   while(true){
      string test;
      cin >> test;
   }
   XPF::millisleep(99999);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Processor::graphify(const string& thefilename){
      
   bool onnodes = false;
   bool onedges = false;
   
   float linkstrengths = 0;
   int linktotal = 0;
   
   int linecounta = 0;
   
   int qualtotal = 0;
   
   textfileloop(thefilename)
   
      linecounta++;
      if(linecounta % 1000 == 0){
         oustoust[220] << "@" >> 240 << linecounta << ozendl;
      }
   
      if(theline == "--phrases--"){
         onnodes = true;
         onedges = false;
      } else if (theline == "--connections--"){
         onedges = true;
         onnodes = false;
      } if(onnodes){
         // actually
         
         vector<string> elements;
         __splitOn(theline, '/', elements);
         
         if(elements.size() == 2){
            Phrase& thephrase = obtainPhrase(elements[0]);
            
            // cool
            vector<string> scores;
            __splitOn(elements[1], ',', scores);
            
            int i;
            for(i = 0; i < scores.size(); i++){
               thephrase.strengthForBucket(i) = __toInt(scores[i]);
            }
            
            thephrase.summarizeStrength();
            
         }
         
      } else if (onedges){
         
         vector<string> elements;
         __splitOn(theline, '/', elements);
         
         if(elements.size() == 3){
            Phrase& phraseone = obtainPhrase(elements[0]);
            Phrase& phrasetwo = obtainPhrase(elements[1]);
            
            PhraseConnection& conn = obtainConnection(phraseone, phrasetwo);
            
            // cool
            vector<string> scores;
            __splitOn(elements[2], ',', scores);
            
            int i;
            for(i = 0; i < scores.size(); i++){
               conn.strengthForBucket(i) = __toInt(scores[i]);
               
            }
            
            conn.summarizeStrength();
            linkstrengths += conn.summarizedstrength;
            linktotal++;
            
            if(conn.summarizedstrength > 20000){
//               Skynet::Edge * edge = processor().skyn.undirectedEdge(phraseone.phrase, "conn", phrasetwo.phrase);
//               if(linktotal % 10 == 0){
//                  XPF::millisleep(100);
//               }
            }
            
         }
         
      }
   textfileloopend
   
   // now, loop through all edges. disable if edge is not on top of the phrases connectivity..
   sloopmapex(phrases, eh, moo)
      moo.calculateTopNConns();
   sloopend
   
   sloopset(allconns, moo)
      if(moo->topNcount == 2){
         // good
         if(moo->summarizedstrength > 100){
            Skynet::Edge * edge = processor().skyn.undirectedEdge(moo->origin->phrase, "conn", moo->target->phrase);
            qualtotal++;
         }
      }
   sloopend
   
   cout << "Qual total : " << qualtotal << endl;
   cout << "Average : " << (linkstrengths / linktotal) << endl;
   cout << "linktotal : " << linktotal << endl;
   
   while(true){
      string test;
      cin >> test;
   }
   XPF::millisleep(99999);
   
   // cool.
   
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Processor& processor(){
   static Processor ret;
   return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
}; // end namespace Gov
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[]){

   CargParser parsa(argc, argv);
   
   if(parsa-- << "graphify"){
      
      string filename = parsa--.parseUnixFileName();

      Gov::processor().loadQueenslandSearches();
      Gov::processor().graphify(filename);
      
      // load out the file
   } else if (parsa-- << "combine"){
      
      // filenames..
      string rest = parsa.parseUntilEnd();
      vector<string> filenames;
      __splitOn(rest, ' ', filenames);
      sloopvec(filenames, moo)
         moo == __chompends(moo);
         oustoust[33] << "Got filename : " >> 240 << moo << ozendl;
      sloopend
      
      Gov::processor().loadQueenslandSearches();
      Gov::processor().combine(filenames);
      
   } else {
   
      int wantedyear = parsa--.parseNumber();
      if(wantedyear < 1870 || wantedyear > 1960){
         cout << "What, give us a good year, please" << endl;
         exit(1);
      }
      
      Gov::processor().loadQueenslandSearches();
      //processor.loadWikiTitles();
      Gov::processor().main(wantedyear);
   }
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
