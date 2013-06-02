//////////////////////////////////////////////////////////////////////////////////////////
#include "trovemap.hpp"
#include "../shing/shinglooper.hpp"
#include "../shing/oust.hpp"
#include "../shing/morestring.hpp"
#include "../shing/once.hpp"
#include "../roengl/utils.hpp"
#include "../roengl/opengldiffs.hpp"
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
namespace TroveMap{
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
bool YearMonth::isNull() const{
   return year == 0 && month == 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
string YearMonth::nicerStr(){
   return SDate::getMonthString(month) + " " + __toString(year);
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
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/*
float& HistoricalRec::strengthForMonth(const YearMonth& themonth){
   return strengthForBucket(bucketForMonth(themonth));
}
*/
//////////////////////////////////////////////////////////////////////////////////////////
ColorInfo HistoricalRec::colorChangeForBucket(int thebucket, bool alphaify){
   float theratio = ratioChangeForBucket(thebucket);
   
   ColorInfo ret;
   
   if(theratio > 1.00f){
      
      float thechange = theratio - 1.0f;
      
      ret = ColorInfo(1.0f, 1.0f - thechange * 0.25f, 1.0f - thechange * 0.5f);
      if(alphaify)
         ret.setAlpha(clampwithin((thechange - 1.0f) / 0.6f, 0, 1));
   } else {
      // blue
      
      ret = ColorInfo(1.0f * theratio, 1.0f * theratio, 1.0f);
      
      if(alphaify)
         ret.setAlpha(1 - theratio);
      
   }
   
   return ret;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
float HistoricalRec::ratioChangeForBucket(int thebucket){
   if(thebucket <= 0)
      return 1;
      
   while(thebucket >= values.size())
      values.push_back(0);
   
   float prevval = values[thebucket - 1];
   if(prevval < 0.001f)
      return 10.0f;
   
   return values[thebucket] / prevval;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
float & HistoricalRec::strengthForBucket(int thebucket){
   while(thebucket >= values.size())
      values.push_back(0);
   
   return values[thebucket];
}
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void PhraseConn::init(Phrase * theone, Phrase * thetwo, TroveKarl * lekarl){
   karl = lekarl;
   one = theone;
   two = thetwo;
   
   one->phraseconns.push_back(this);
   two->phraseconns.push_back(this);
   
}
////////////////////////////////////////////////////////////////////////////////////////
void PhraseConn::syncRVA(RVA& linerva, Graph& legraph){
   
   linerange.from = linerva.size();
   
   float strength = 0.14f;
   if(!legraph.ym.isNull()){
      strength = signedpow(strengths.strengthForBucket(legraph.bucketnum), 0.5f) * 0.03f;
      
      ColorInfo lecol = strengths.colorChangeForBucket(legraph.bucketnum).atAlpha(strength);
      linerva.addLine(one->pos, two->pos, lecol); // simplemente
   } else {
      linerva.addLine(one->pos, two->pos, ColorInfo(1,1,1,strength)); // simplemente
   }
   
   
   
   linerange.to = linerva.size();
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void PhraseConn::syncFocusEdgeRVA(RVA& rva){
   
   Coord3D theup(0, 1, 0); // let's just say that is the case
   Coord3D dirnorm = (one->pos - two->pos).normalise();
   Coord3D cross1 = cross(theup, dirnorm).normalise();
   Coord3D cross2 = cross(cross1, dirnorm).normalise();
   
   cross1 = cross1 * 0.03f;
   cross2 = cross2 * 0.03f;
   
   rva.addSquarePolygon(one->pos + cross1, two->pos + cross1, two->pos - cross1, one->pos - cross1);
   rva.addSquarePolygon(one->pos + cross2, two->pos + cross2, two->pos - cross2, one->pos - cross2);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
Phrase * PhraseConn::getOtherSide(Phrase * moo){
   if(moo == one)
      return two;
   
   return one;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void Phrase::init(const string& thename, TroveKarl * thekarl){
   name = thename;
   karl = thekarl;
}
//////////////////////////////////////////////////////////////////////////////////////////
float Phrase::getScreenTapScore(const Point2D& thepos){
   Coord3D screenpos = karl->marsa.worldCoordToScreenCoord3D(pos);

   if(screenpos.z > 0)
      return -1;
      
   Point2D twodpart = screenpos.get2DPart();
   float len = twodpart.lengthFrom(thepos);
   if(len > 25)
      return -1;
   
   return (40 - len) + 50.0f / max(-screenpos.z, 1.0f);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Phrase::gatherNeighbors(vector<Phrase*>& ret){
   sloopvec(phraseconns, moo)
      Phrase * rhs = moo->getOtherSide(this);
      ret.push_back(rhs);
   sloopend
}
//////////////////////////////////////////////////////////////////////////////////////////
void Phrase::syncFocusEdgeRVA(RVA& dotsrva){
   sloopvec(phraseconns, moo)
      moo->syncFocusEdgeRVA(dotsrva); // yeah
   sloopend
}
//////////////////////////////////////////////////////////////////////////////////////////
void Phrase::syncRVA(RVA& dotsrva, Graph& legraph){
   
   float strength = 1.0f;
   if(!legraph.ym.isNull()){
      strength = signedpow(strengths.strengthForBucket(legraph.bucketnum) / 50.0f, 0.4f) * 1.0f;
      strength = max(0.1f, strength);
   }
   
   float dd = 0.05f; // yeah
   
   ColorInfo c(1,0,0);
   if(!legraph.ym.isNull()){
      c = strengths.colorChangeForBucket(legraph.bucketnum).atAlpha(strength);
   } else {
   
      if(country)
         c = ColorInfo(1,1,0);
      else if (isname)
         c = ColorInfo(1,0.2f,0.6f);
   }
      
   dd *= strength;
   
   range_dottotal.from = dotsrva.size();
   
   range_dot1.from = dotsrva.size();
   
   dotsrva.addSquarePolygon(pos + Coord3D(-dd, 0, 0), pos + Coord3D(0, -dd, 0),
                             pos + Coord3D(dd, 0, 0), pos + Coord3D(0, dd, 0),
                              c,c,c,c);
   
   range_dot1.to = dotsrva.size();
   
   
   range_dot2.from = dotsrva.size();
   
   dotsrva.addSquarePolygon(pos + Coord3D(0, -dd, 0), pos + Coord3D(0, 0, -dd),
                             pos + Coord3D(0, dd, 0), pos + Coord3D(0, 0, dd),
                              c,c,c,c);
   
   range_dot2.to = dotsrva.size();
   
   range_dot3.from = dotsrva.size();
   
   dotsrva.addSquarePolygon(pos + Coord3D(-dd, 0, 0), pos + Coord3D(0, 0, -dd),
                             pos + Coord3D(dd, 0, 0), pos + Coord3D(0, 0, dd),
                              c,c,c,c);
   
   range_dot3.to = dotsrva.size();
   
   range_dottotal.to = dotsrva.size();
   
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConnIndex::PhraseConnIndex(){
   one = NULL;
   two = NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConnIndex::PhraseConnIndex(Phrase * leone, Phrase * letwo){
   if(leone < letwo){
      one = leone;
      two = letwo;
   } else {
      one = letwo;
      two = leone;
   }
}
//////////////////////////////////////////////////////////////////////////////////////////
bool PhraseConnIndex::operator<(const PhraseConnIndex& rhs) const{
   if(one < rhs.one)
      return true;
   else if (one > rhs.one)
      return false;
   
   return two < rhs.two;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
NodeProofer::NodeProofer(TroveKarl * lekarl, Page * thepage):
   ProoferBase("nodeproofer", thepage){
   
   karl = lekarl;
   
   prooferrowcount = 6;
   
   midbit = mIC();
      
      prooferCns(220, '6', midbit);
      
      opensearchbutton = prooferButton("journbadges_0175.png", 1.0f, 64, '8', midbit);
      unfocusbutton = prooferButton("small32_down.png", 2.0f, 64, '2', midbit);
      
      prooferCns(220, '4', midbit);
   
   dynamicx = false;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void NodeProofer::open(const Point2D& screenpos){
   DynaRect rect;
   rect.specify(0, 0, 128, 88);
   rect.shiftOriginToCenter();
   rect.setCoord(roengl->pagemid().x, screenpos.y);
   
   proof(rect); // yeah
}
//////////////////////////////////////////////////////////////////////////////////////////
void NodeProofer::onProoferTap(Cns * thebutton){
   if(thebutton == opensearchbutton){
   
      // well
      if(karl->focusphrase != NULL){
         // open the URL         
         string yearfrom = __toString(karl->baseym.year);
         string monthfrom = Morestring::zeroedNumber(karl->baseym.month, 2);
         
         string yearto = __toString(karl->lastym.year);
         string monthto = Morestring::zeroedNumber(karl->lastym.month, 2);
         
         if(karl->activegraph->ym.isNull()){
            // meh
         } else {
            yearfrom = yearto = __toString(karl->activegraph->ym.year);
            monthfrom = monthto = Morestring::zeroedNumber(karl->activegraph->ym.month, 2);
         }
         
         string theurl = "http://trove.nla.gov.au/newspaper/result?q="
                     + Morestring::urlencode(karl->focusphrase->name) +
                     "&exactPhrase=&anyWords=&notWords=&l-textSearchScope=*ignore*%7C*ignore*&fromdd=&frommm=" + monthfrom + "&fromyyyy=" + yearfrom + 
                     "&todd=&tomm=" + monthto + "&toyyyy=" + yearto + "&l-title=%7C35&l-category=Advertising%7Ccategory%3AAdvertising&l-category=Article%7Ccategory%3AArticle&l-category=Detailed+lists%2C+results%2C+guides%7Ccategory%3ADetailed+lists%2C+results%2C+guides&l-category=Family+Notices%7Ccategory%3AFamily+Notices&l-category=Literature%7Ccategory%3ALiterature&l-word=*ignore*%7C*ignore*&sortby=";
         
         MacStuff::loadURL(theurl); // yeah, this will have to do for now
         
      }
   
   } else if (thebutton == unfocusbutton){
      karl->focus(NULL);
   }
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
PhraseLabeler::PhraseLabeler(TroveKarl * thekarl):
   tdc("v20"){
   karl = thekarl;
   
   phrase = NULL;
   instaposflag = false;
   ismain = false;
   infront = false;
   
   tdc.setBacker("deco_darkthickborder.png", 0.4f, 8);
   tdc.setColor(1);
   
   if(__isIPad())
      tdc.setScale(2.0f); // ipad, yeah
   
}
//////////////////////////////////////////////////////////////////////////////////////////
bool PhraseLabeler::handleTap(const Point2D& thepos){
   if(phrase == NULL)
      return false;
   
   if(!infront)
      return false;
   
   // now, based on tdc.pos
   if(tdc.contains(thepos, 10)){
      
      if(ismain){
         // already being focused, yep
         // call up the proofer
         karl->nodeproofer->open(thepos);
      } else {
         karl->focus(phrase);
      }
      
      return true;
   }
   
   return false;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void PhraseLabeler::imprint(Phrase * thephrase, bool asmain){
   phrase = thephrase;
   
   if(thephrase != NULL)
      tdc.setText(thephrase->name); // yeah
      
   ismain = asmain;
      
   if(!asmain){
      tdc.setFont("v16");
      tdc.setBackerColor(ColorInfo(0,0,0, 0.3f));
      tdc.setColor(ColorInfo(1));
   } else {
      tdc.setFont("v20");
      tdc.setBackerColor(ColorInfo(0.4f));
      tdc.setColor(ColorInfo(1,1,1,0.5f));
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void PhraseLabeler::syncScreenPos(){
   
   if(phrase == NULL)
      return;
   
   Coord3D lepos = karl->marsa.worldCoordToScreenCoord3D(phrase->pos);
   screenposchase = lepos.get2DPart();
   
   infront = lepos.z < 0;
   // meh, for now
   
   if(ismain){
      tdc.setPos(screenposchase.val + Point2D(0, 40)); // yeah
   } else {
      tdc.setPos(screenposchase.val); // yeah
   }
   instaposflag = false;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void PhraseLabeler::draw(){
   
   if(instaposflag || phrase == NULL || !infront)
      return; // don't, pos is not proper
   
   tdc.update();
   tdc.draw(); // yeah
   
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
SearchPad::SearchPad(TroveKarl * lekarl, Page * thepage){
   
   page = thepage;
   karl = lekarl;
   
   midbit = mIC();
   
      editchild = midbit->mEC(this);
         editchild->addText("")->greedyXer();
         editchild->setMinimumSize(90, 36);
         editchild->setEditFontSize(20);
         editchild->setEditGustos("invisi", "border8");
         editchild->addEmptyPlaceholder("");
      
      erasebutton = midbit->mBC(this)->setGustos("invisi")->setIcon("small32_left.png", 1.0f)->setMinimumSize(10, 36);
   
   setDepth(160);
   changeLength(490);
   setLocation(page->roengl->pagemid() + Point2D(-490 / 2, 0));
   sink(0);
   
   activexnld = true; // why not
   deriveActiveness(activexnld);
   regSelf(page);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void SearchPad::delegatedOnRelease(const TouchInfo& tinfoil, Cns * origin){
   if(origin == erasebutton){
      existingsearch = "";
      editchild->changeText(0, "");
   }
}
//////////////////////////////////////////////////////////////////////////////////////////
void SearchPad::update(){
   if(editchild->hasKeyboardFocus()){
      
      if(editchild->getKeyboardFocusBuffer() != lasttextboxentry){
         lasttextboxentry = editchild->getKeyboardFocusBuffer();
         
         if(Morestring::contains(lasttextboxentry, "\n")){
            
            // well
            vector<string> elements;
            __splitOn(lasttextboxentry, '\n', elements);
            lasttextboxentry = elements[0];
            roengl->setKeyboardBuffer(lasttextboxentry);
            // and..
            editchild->releaseKeyboardInput(); // yeah
            
            cout << "Searching : " << lasttextboxentry << endl;
            
            karl->onSearchEnterNode(lasttextboxentry);
            
         }
         
      }
      
   }
}
//////////////////////////////////////////////////////////////////////////////////////////
void SearchPad::onEditStart(KeyboardFocus& kb, Cns * origin){
   // nothing
   
   kb << existingsearch;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void SearchPad::onEditFinish(const string& buffercontents, KeyboardLoss& loss, Cns * origin){
   // well, yeah
   // cool
   cout << "Did this get called at least" << endl;
   
   editchild->changeText(0, editchild->getKeyboardFocusBuffer());
   existingsearch = buffercontents;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
TimelineSlider::TimelineSlider(TroveKarl * lekarl, Page * thepage){
   karl = lekarl;
   phrase = NULL;
   page = thepage;
   
   areabit = mIC()->setAsButton();
   areabit->inputDelegate = this;
   areabit->setMinimumSize(__roengl()->viewwidth, 60); // yeah
   
   setGustos("blackblock", "{bgcols:#0009}", "border0");
   
   setDepth(160);
   changeLength(__roengl()->viewwidth);
   setLocation(page->roengl->pagemid() + Point2D(- __roengl()->viewwidth / 2, 0));
   sink(0);
   
   activexnld = true; // why not
   deriveActiveness(activexnld);
   regSelf(page);
   
   synced = false;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TimelineSlider::sync(){
   
   if(synced)
      return;
      
   synced = true;
   
   backer.reset();
   
   int totalwidth = __roengl()->viewwidth;
   int height = 60; // yep
   
   int i;
   for(i = 0; i < karl->bucketcount; i++){
      // well, draw the nth bucket
      
      int xstart = (i * totalwidth) / karl->bucketcount;
      int xend = ((i + 1) * totalwidth) / karl->bucketcount;
      
      if(phrase == NULL){
      
         ColorInfo touse;
         if(i % 2 == 0){
            touse = ColorInfo(0.5f, 0.5f, 0.5f, 0.5f);
         } else {
            touse = ColorInfo(0.3f, 0.3f, 0.3f, 0.5f);
         }
         
         backer.addRectangle2D(xstart, 0, xend - xstart, 60, touse); // yeah
         
      } else {
         // well, depends on focus phrase's concentration, etc
         // beh
         ColorInfo topcol = signedpow(phrase->strengths.strengthForBucket(i) / 50.0f, 0.3f) * 0.2f;
         ColorInfo botcol = phrase->strengths.colorChangeForBucket(i, true);
         
         backer.addRectangle2D(xstart, 0, xend - xstart, 40, topcol); // yeah
         backer.addRectangle2D(xstart, 20, xend - xstart, 20, botcol); // yeah
         
      }
      
   }
   // how many buckets? let's assume 10
   // or something
   // cool, right?
}
//////////////////////////////////////////////////////////////////////////////////////////
void TimelineSlider::drawAfterChild(const CDrawArg& arg){
   
   glPushMatrix();
   
   glTranslatef(arg.pos.getRealLeft(), arg.pos.getRealTop(), 0);
   glDisable(GL_TEXTURE_2D);
   backer.drawTriangles();
   
   // if there's an active thing..
   
   static int lastbucketnum = -1;
   
   if(karl->activegraph->bucketnum >= 0){
      static TDC tdc("s8");
      onlyonce(
      
         if(__isIPad())
            tdc.setScale(2.0f);
         tdc.setAlignment(-1, 0);
         tdc.setBacker("deco_darkthickborder.png", 0.4f, 8);
         tdc.setBackerColor(ColorInfo(0.4f));
         tdc.setColor(ColorInfo(1,1,1,0.5f));
         tdc.setRot(-90); // yeah, let's try this
      );
      
      if(lastbucketnum != karl->activegraph->bucketnum){
         lastbucketnum = karl->activegraph->bucketnum;
         tdc.setText(karl->activegraph->ym.nicerStr());
         float xpos = (__roengl()->viewwidth * ((lastbucketnum + 0.5f) / (float)karl->bucketcount));
         float ypos = -20;
         tdc.setPos(Point2D(xpos, ypos));
      }
      
      tdc.update();
      tdc.draw();
      
   }
   
   glPopMatrix();
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TimelineSlider::update(){
   
   if(phrase != karl->focusphrase){
      phrase = karl->focusphrase;
      synced = false;
   }
   
   sync();
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TimelineSlider::delegatedOnPress(const TouchInfo& tinfoil, Cns * origin){
   claimTouch(tinfoil); // yeah
   handleTouchEvent(tinfoil);
}
//////////////////////////////////////////////////////////////////////////////////////////
bool TimelineSlider::handleTouchEvent(const TouchInfo& tinfoil){
   
   DynaRect rect = getRectOnScreen();
   
   if(tinfoil.isDown() || tinfoil.isMove()){
      
      if(rect.contains(tinfoil.x, tinfoil.y)){
         int xindex = (karl->bucketcount * (tinfoil.x - rect.getRealLeft())) / rect.getWidth();
         if(xindex <= 0)
            xindex = 0;
         if(xindex >= karl->bucketcount)
            xindex = karl->bucketcount - 1;
         
         karl->activateBucket(xindex);
      } else {
         karl->activateBucket(-1);
      }
      
   } else {
      if(rect.contains(tinfoil.x, tinfoil.y)){
         // all cool
      } else {
         karl->activateBucket(-1);
      }
   }
   
   return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Graph::Graph(){
   rvasynced = false;
   bucketnum = -1;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
TroveKarl::TroveKarl(Page * thepage):
   RoenObject(__roengl()),
   KeyboardHandler(__roengl()),
   marsa(__roengl()),
   runningmodel("sninjarun", 10, 30),
   Karlet(thepage){

   page = thepage;
   
   baseym.year = 1942;
   baseym.month = 1; // yeah
   bucketcount = 10; // minimum num
   lastym = baseym.monthNeigh(bucketcount - 1);
   
   lookdist = 1;
   lookdistmaster = 1;
   tapmatch_need = false;
   
   focusphrase = NULL;
   
   no_perspective = true;
   shoulddrawagent = false;
   
   activegraph = &globalgraph; // yeah, for now.
   
   runningmodel.setTex("mcs_sn.png");
   
   //ColorInfo backercol("003349");
   ColorInfo backercol("111111");
   
   marsa.setDelegate(this);
   marsa.enableFog(backercol);
   marsa.setFogDistances(1.5f, 39.0f);
   
   //marsa.setSkybox("bluesky2", "jpg");
   marsa.enableBackgroundTinting(backercol);
   
   //importGraph();
   importGraphBig();
   
   marsa.cam->setChaseSpeed(0.3f, 0.3f, 0.3f);
   
   enableCustomLayer(true);
   
   int i;
   for(i = 0; i < 20; i++){
      labelers.push_back(new PhraseLabeler(this));
   }
   
   searchpad = new SearchPad(this, page);
   slider = new TimelineSlider(this, page);
   nodeproofer = new NodeProofer(this, page);
   
   __roengl()->markMoopadPref(-1);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::onSearchEnterNode(const string& nodename){
   
   searchvec.clear();
   
   sloopvec(phrasevec, moo)
      
      if(Morestring::contains(moo->name, nodename)){
         searchvec.push_back(moo);
      }
      
   sloopend
   
   if(searchvec.size() == 1){
      focus(searchvec[0]);
   } else if (searchvec.size() == 0){
      __openNoti("Doesn't look like that phrase was talked about much in this period..");
   } else {
      requestCommand("search");
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::handleSearchCommand(Command& comm){
   commando_start
      
      sloopvec(searchvec, moo)
         staga.addChoiceMarkup(moo->name, "[v20]" + moo->name);
      sloopend
      
      staga.addChoiceMarkup("__cancel", "[v15]cancel").setGustos("plainbordergrey").atbottom();
      
   commando_case("__cancel")
      // meh
   commando_else
      
      Phrase * thephrase = maybeGetPhrase(comm.curWord());
      if(thephrase != NULL)
         focus(thephrase);
      
   commando_end
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::handleCommand(Command& comm, CommandStatus& cstat){
   
   if(__isIPad())
      comm.commSettings().customScale(2.0f);
   
   if(comm.mode == "search"){
      return handleSearchCommand(comm);
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::setBaseYear(int year){
   assert(graphbuckets.size() == 0);
   baseym.year = year;
   baseym.month = 1; // yep
}
//////////////////////////////////////////////////////////////////////////////////////////
Graph * TroveKarl::obtainGraph(int bucketnum){
   while(bucketnum > graphbuckets.size()){
   
      int bucketindex = graphbuckets.size();
   
      Graph * thegraph = new Graph();
      graphbuckets.push_back(thegraph); // yep
      
      thegraph->bucketnum = bucketindex;
      thegraph->ym = baseym.monthNeigh(bucketindex);
      
      bucketcount = max(bucketcount, (int)graphbuckets.size());
      lastym = baseym.monthNeigh(bucketcount - 1);
      
   }
   
   return graphbuckets[bucketnum];
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::activateBucket(int thebucketnum){
   if(thebucketnum < 0){
      activegraph = &globalgraph;
   } else if(thebucketnum >= 0 && thebucketnum < graphbuckets.size()){
      activegraph = graphbuckets[thebucketnum];
   } else {
      activegraph = &globalgraph; // beh
   }
}
//////////////////////////////////////////////////////////////////////////////////////////
bool TroveKarl::handleKeyPressRepeat(const RoenKeyData& rkd, float secssincepressed){
   
   if(rkd.code == rk_w){
      lookdist.chase2 -= 0.1f;
      lookdist.chase2 = clampwithin(lookdist.chase2, 0.5f, 50.0f);
      return true;
   } else if (rkd.code == rk_a){
      return true;
   } else if (rkd.code == rk_s){
      lookdist.chase2 += 0.1f;
      lookdist.chase2 = clampwithin(lookdist.chase2, 0.5f, 50.0f);
      return true;
   } else if (rkd.code == rk_d){
      return true;
   }
   
   return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConn& TroveKarl::obtainConn(Phrase& one, Phrase& two){
   return obtainConn(&one, &two);
}
//////////////////////////////////////////////////////////////////////////////////////////
PhraseConn& TroveKarl::obtainConn(Phrase * one, Phrase * two){
   PhraseConnIndex index(one, two);
   
   auto it = conns.find(index);
   if(it == conns.end()){
      PhraseConn& theconn = conns[index];
      // and init it
      theconn.init(one, two, this);
      return theconn; // yep
   }
   
   return it->second;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
Phrase * TroveKarl::maybeGetPhrase(const string& thephrase){
   Phrase ** ret = phrasetrie.maybeGet(thephrase);
   if(ret == NULL)
      return NULL;
   
   return *ret; // .. eh
}
//////////////////////////////////////////////////////////////////////////////////////////
Phrase& TroveKarl::obtainPhrase(const string& thephrase){
   
   if(phrasetrie.count(thephrase) == 0){
      Phrase& ref = phrases[thephrase];
      ref.init(thephrase, this);
      phrasetrie[thephrase] = &ref;
      phrasevec.push_back(&ref);
      return ref;
   } else {
      return *phrasetrie[thephrase];
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::importGraphBig(){

   
   bool onnodes = false;
   bool onedges = false;
   
   setBaseYear(1939);
   
   int maxbucket = 0;
   
   textfileloopmac("finalgraph.out")
      
      if(Morestring::beginsWith(theline, "yearbegin:")){
         
         EasyParser parsa(theline);
         parsa << "yearbegin:";
         int theyear = parsa.parseNumber();
         assert(theyear > 1900 && theyear < 2000);
         setBaseYear(theyear);
      } else if(theline == "---nodes---"){
         onnodes = true;
         onedges = false;
      } else if (theline == "---edges---"){
         onnodes = false;
         onedges = true;
      } else if (onnodes){
         
         vector<string> elts;
         __splitOn(theline, '|', elts);
         if(elts.size() == 3){
            Phrase& lephrase = obtainPhrase(elts[0]);
            
            EasyParser flagparsa(elts[1]);
            if(flagparsa << 'c')
               lephrase.country = true;
            if(flagparsa << 'n')
               lephrase.isname = true;
            
            // flag time
            vector<string> strengthvals;
            __splitOn(elts[2], ',', strengthvals);
            int counta = 0;
            sloopvec(strengthvals, moo)
               lephrase.strengths.strengthForBucket(counta) = Morestring::toFloat(moo);
               counta++;
            sloopend
            
            maxbucket = max(maxbucket, (int)strengthvals.size());
            
         }
         
      } else if (onedges){
         
         
         vector<string> elts;
         __splitOn(theline, '|', elts);
         if(elts.size() == 3){
            Phrase& lephrase = obtainPhrase(elts[0]);
            Phrase& phrasetwo = obtainPhrase(elts[1]);
            
            PhraseConn& conn = obtainConn(lephrase, phrasetwo);
            
            // flag time
            vector<string> strengthvals;
            __splitOn(elts[2], ',', strengthvals);
            int counta = 0;
            sloopvec(strengthvals, moo)
               conn.strengths.strengthForBucket(counta) = Morestring::toFloat(moo);
               counta++;
            sloopend
            
            maxbucket = max(maxbucket, (int)strengthvals.size());
            
         }
         
      }
   
   textfileloopend
   
   
   textfileloopmac("graphexport.out")
      
      if(theline == "---nodes---"){
         onnodes = true;
         onedges = false;
      } else if (theline == "---edges---"){
         onnodes = false;
         onedges = true;
         break; // who cares about the rest
      } else if (onnodes){
      
         vector<string> lesplit;
         __splitOn(theline, '|', lesplit);
         
         if(lesplit.size() == 2){
            Phrase& lephrase = obtainPhrase(lesplit[0]);
            EasyParser parsa(lesplit[1]);
            
            lephrase.pos.x = parsa.parseFloat();
            parsa << ',';
            lephrase.pos.y = parsa.parseFloat();
            parsa << ',';
            lephrase.pos.z = parsa.parseFloat();
            
         }
      
      }
      
   textfileloopend
   
   obtainGraph(maxbucket - 1); // yeah
   
   // sync lines, etc
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::importGraph(){
   
   bool onnodes = false;
   bool onedges = false;
   
   setBaseYear(1939);
   obtainGraph(12 * 8); // why not
   
   textfileloop("/home/bennylava/temp2/graphpos_1945.out")
      
      if(theline == "---nodes---"){
         onnodes = true;
         onedges = false;
      } else if (theline == "---edges---"){
         onnodes = false;
         onedges = true;
      } else if (onnodes){
      
         vector<string> lesplit;
         __splitOn(theline, '|', lesplit);
         
         if(lesplit.size() == 2){
            Phrase& lephrase = obtainPhrase(lesplit[0]);
            EasyParser parsa(lesplit[1]);
            
            lephrase.pos.x = parsa.parseFloat();
            parsa << ',';
            lephrase.pos.y = parsa.parseFloat();
            parsa << ',';
            lephrase.pos.z = parsa.parseFloat();
            
         }
      
      } else if (onedges){
         
         vector<string> lesplit;
         __splitOn(theline, '|', lesplit);
         
         if(lesplit.size() == 2){
            Phrase& one = obtainPhrase(lesplit[0]);
            Phrase& two = obtainPhrase(lesplit[1]);
            
            obtainConn(one, two);
            
         }
         
      }
      
   textfileloopend
   
   // sync lines, etc
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::syncGraphRVA(){
   if(activegraph->rvasynced)
      return;
   
   sloopmapex(phrases, eh, moo)
      moo.syncRVA(activegraph->dotsrva, *activegraph);
   sloopend
   
   sloopmapex(conns, eh, moo)
      moo.syncRVA(activegraph->linesrva, *activegraph); // yeah..
   sloopend
   
   activegraph->dotsrva.syncVBO();
   activegraph->linesrva.syncVBO();
   
   activegraph->rvasynced = true;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::onMarsDrawPostUnitSolids(){
   
   
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::onMarsDrawPreUnitSolids(){
   
   syncGraphRVA();
   
   GLEnabler blenda(GL_BLEND);
   
   // let's try draw outlines
   
   float outlinethickness = 2;
   
   glLineWidth(outlinethickness);
   glDisable(GL_TEXTURE_2D);
   
   activegraph->dotsrva.drawTriangles();
   activegraph->linesrva.drawLinesNoWidth();
   
   if(focusphrase != NULL){
      glColor4f(1,1,1,0.2f + __roenosci2 * 0.1f);
      focusrva.drawTriangles();
   }
   
   sloopvec(labelers, moo)
      moo->syncScreenPos();
   sloopend
   
   if(tapmatch_need){
   
      float bestscore = -2;
      Phrase * topphrase = NULL;
   
      sloopvec(phrasevec, moo)
         // get tap scoreness
         float thescore = moo->getScreenTapScore(tapmatch_pos);
         if(thescore > bestscore){
            topphrase = moo;
            bestscore = thescore;
         }
      sloopend
      
      if(topphrase != NULL && bestscore > 0){
         focus(topphrase);
      }
      
      tapmatch_need = false;
      
   }
   
   // while on this modelview..
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::drawCustomLayer(){
   
   // well
   // Utils::blackout(ColorInfo(0,0,0.2f)); // let's try this
   marsa.draw();
   
   sloopvecrev(labelers, moo)
   
      moo->draw();
      
   sloopend
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::focus(Phrase * lephrase){

   if(lephrase == focusphrase)
      return;

   focusphrase = lephrase;
   
   if(focusphrase != NULL){
      midcam.chase2 = focusphrase->pos;
   } else {
      midcam.chase2 = Coord3D();
   }
   
   sloopvec(labelers, moo)
      moo->imprint(NULL);
   sloopend
   
   focusrva.reset();
   if(focusphrase != NULL){
      focusphrase->syncFocusEdgeRVA(focusrva); // yeah
   
      vector<Phrase*> toimprint;
      toimprint.push_back(focusphrase);
      
      focusphrase->gatherNeighbors(toimprint);
      
      int i;
      for(i = 0; i < toimprint.size(); i++){
         if(i >= labelers.size())
            break;
         
         labelers[i]->imprint(toimprint[i], i == 0);
         
         // yeah
         
      }
      
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::updateAsFocus(){
   
   {
      static int counta = 0;
      counta++;
      if(counta == 50){
         // and select random focus
         
         Phrase * aust = maybeGetPhrase("australia");
         focus(aust);
         
         /*
         int therandindex = intrandbetween(0, phrasevec.size());
         Phrase * lephrase = phrasevec[therandindex];
         
         focus(lephrase);
         */
         
         // counta = 0;
      }
   }
   
   
   static Angle360 rotterangle;
   rotterangle += 0.3f;
   
   xviewangle.chaseByFraction(0.2f, __ldtf);
   
   Point2D theboo = Point2D::modarg(degreestoradians(xviewangle.val.value), 1.0f);
   
   yviewshift.chaseByFraction(0.2f, __ldtf);
   
   Coord3D lookdir(-theboo.x, yviewshift.val * 0.01f, -theboo.y);
   lookdist.chaseByFraction(0.15f, __ldtf);
   
   lookdistmaster.chase2 = lookdist.val;
   
   if(focusphrase == NULL)
      lookdistmaster.chase2 *= 3.4f;
      
   lookdistmaster.chaseByFraction(0.2f, __ldtf);
   lookdir *= lookdistmaster.val;
   
   if(focusphrase != NULL){
      fogfar.chase2 = 39;
   } else {
      fogfar.chase2 = 90;
   }
   fogfar.chaseByFraction(0.2f, __ldtf);
   marsa.setFogDistances(1.5f, fogfar.val);
   
   midcam.chaseByFraction(0.15f, __ldtf);
   Coord3D looktarg = midcam.val;
   
   marsa.cam->fixedLook(looktarg - lookdir, looktarg, Coord3D(0,1,0));
   //marsa.cam->fixedLook(looktarg - lookdir, looktarg, Coord3D(0,0,1));
   
   float camchasespeed = 0.7f;
   
   marsa.cam->setChaseSpeed(camchasespeed,camchasespeed,camchasespeed);
   
   runningmodel.animZTO(fmod(__roentimestamp, 1));
   
   marsa.updateState(); // yep, why not
   
}
//////////////////////////////////////////////////////////////////////////////////////////
bool TroveKarl::prehandleTouchEvent(const TouchInfo& tinfoil){
   
   if(tinfoil.flagged("hasteout")){
      return false; // meh
   }
   
   // well, just claim it
   claimTouch(tinfoil);
   handleTouchEvent(tinfoil);
   
   return true;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void TroveKarl::processSimpleTap(const Point2D& thepos){
   // first, check if any of the labels hit
   
   sloopvec(labelers, moo)
      if(moo->handleTap(thepos)){
         return;
      }
   sloopend
   
   // well, shit, no match
   tapmatch_pos = thepos;
   tapmatch_need = true;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
bool TroveKarl::handleTouchEvent(const TouchInfo& tinfoil){
   
   
   if(tinfoil.isDown()){
      
      // now, should be claimed by now
      if(claimedTouchCount() > 1){
         vector<TouchInfo> alltouches = getAllClaimedTouches();
         sloopvec(alltouches, moo)
            moo.flag("zoom"); // yeap, zoom mode active
         sloopend
      }
   
   } else if(tinfoil.isMove()){
      
      if(tinfoil.flagged("zoom")){
         
         if(claimedTouchCount() > 1){
            
            TouchInfo othertouch = getTheOtherClaimedTouch(tinfoil);
            float olddist = tinfoil.pos().lengthFrom(othertouch.pos());
            float newdist = (tinfoil.pos() + tinfoil.dpos()).lengthFrom(othertouch.pos());
            
            if(olddist > 0.01f && newdist > 0.01f){
               float multratio = newdist / olddist;
               lookdist.chase2 /= multratio; // yeah
               lookdist.chase2 = clampwithin(lookdist.chase2, 0.5f, 50.0f);
            }
            
         }
         
      } else {
         // let's see
         xviewangle.chase2 += tinfoil.dx;
         yviewshift.chase2 += -tinfoil.dy;
      }
      
   } else if (tinfoil.isUp()){
   
      if(tinfoil.flagged("zoom")){
         // meh
      } else if(!tinfoil.didMove(8) && tinfoil.sinceTouchDown() < 0.5f){
         // simple tap!
         processSimpleTap(tinfoil.pos());
      }
   }
   
   return true;
   
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
}; // end namespace TroveMap
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
