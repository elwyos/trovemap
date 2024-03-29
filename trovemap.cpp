//////////////////////////////////////////////////////////////////////////////////////////
#include "trovemap.hpp"
#include "../shing/shinglooper.hpp"
#include "../shing/oust.hpp"
#include "../shing/morestring.hpp"
#include "../shing/once.hpp"
#include "../shing/flector.hpp"
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
GameEnv * createGame(RoenGL * roen, int width, int height){
   return NULL;
}
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
RegisterStandardPage(Page, 10);
//////////////////////////////////////////////////////////////////////////////////////////
Page::Page(RoenGL * roen):
   savedir(roen->getSaveDirectory("proker")),
   KarPage("proker", roen){
   
   claimURL("spelm");
   roengl->declareSpritePack("rpg_items.png");
   
   roengl->setupPronouncerLanguage("english");
   roengl->setupVoice("melanie");
   
   appcomm().appName("proker")
            .appLogo("journbadges_0074.png")
            .topDesc("[v20]<250>Main Menu\n<235>--------------------")
            .tipsFile("")
            .helpID("proker");
   //roengl->declareSpritePack("fuguecal.png");
   topui_banner_icon = "journbadges_0074.png";

   // that's it for now!
   
   hideTop(true);
   hideBottom(true);
   allowsbottominterceptor = false;
   
   // - - - - - - - - - - - - - - - - - -   
   
   roengl->models.loadSpecsFromFile("humanmodels.cyano"); // yep, why not
   
   // - - - - - - - - - - - - - - - - - -   
   
   trovekarl = new TroveKarl(this);
   
   // - - - - - - - - - - - - - - - - - -
   
   flector_defstart(2)
   flector_defend
   
   load();
   
   // - - - - - - - - - - - - - - - - - -
   
   initSignols();
   
   onRoenBecomeActive();
   
   enableStatusBar(false);
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::load(){

   flector_roenobscureloadfile(savedir + "save.flec");
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::resetState(){
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::performSave(){

   flector_roenobscuresavefile(savedir + "save.flec");
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::initSignols(){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::onActivate(Xnl& moo){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::onDeactivate(Xnl& moo){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::onRoenBecomeActive(){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::onRoenResignActive(){
   maybeSaveLazy();
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::karpage_onDayChange(){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::karpage_updateHalfsec(){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::karpage_update(){

   maybeSave();

}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::updateFiveSecs(){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::onFocusKarletChange(Karlet * thekarl){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::handleMenuCommand(Command& comm){
   
   maybeSave();
   
   commando_start
   
      staga.addChoiceMarkup("--", "{journbadges_0011.png : +#222 @ 1.0}").setGustos("invisi").lock();
      
      staga.addChoiceMarkup("help", "[v20]Help");
      
      staga.addChoiceMarkup("upgrade", "[v20]Upgrade To Pro"); // freemium stuff
      
      staga.addChoiceMarkup("options", "[v15]Options, Setttings, Credits etc");
   
   commando_case("showendstats")
   
   commando_case("resumegame")
   
   commando_case("newgame")
   
   commando_case("restartround")
   
   commando_case("help")
   
   commando_case("upgrade")
   
   commando_case("options")
   
   commando_end
   
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::handleCommand(Command& comm, CommandStatus& cstat){
   
   if(comm.mode == "menu"){
      return handleMenuCommand(comm);
   }
   
}
//////////////////////////////////////////////////////////////////////////////////////////
bool Page::karpage_handleTouchBeforeTopbots(const TouchInfo& tinfoil){
   return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::commlister_list(const string& listname, CommlistItems& items){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::commlister_listfields(const string& listname, const string& itemid, CommlistFields& fields){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::commlister_set(const string& listname, const string& itemid, CommlistFields& fieldvals){
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::commlister_delete(const string& listname, const string& itemid){
}
//////////////////////////////////////////////////////////////////////////////////////////
string Page::commlister_headermarkup(const string& listname){
//   if(listname == "calendar"){
//      return "{fastminder_toplogo.png @ 0.5}\n[v20]<244>Calendars";
//   }
   return "";
}
//////////////////////////////////////////////////////////////////////////////////////////
bool Page::commlister_checkAllowItemAdding(const string& listname){
   return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Page::handleURL(const string& method, EasyParser& parsa){
   // beh
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
}; // end namespace Spellmancer
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
