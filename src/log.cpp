
#include <filesystem>
#ifdef _WIN32
#include "winapi.h"
#include <windows.h>
#endif
#include "log.h"
#include "util.h"

/*
 Explications de la journalisation
Les différents niveaux de journalisation possibles, dans leur ordre de priorité, sont :
  ALL < TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF

Comment la journalisation s'effectue :
(en partie inspirés de log4j)
  ALL   tous les messages.
  TRACE presque tous les messages, en tout cas ceux plus fins que le niveau DEBUG.
  DEBUG évenements à granularité fine les plus utiles pour déboguer une application.
  INFO  messages d'information qui mettent en évidence la progression de l'application à un niveau à granularité grossière.
  WARN  situations potentiellement dangereuses.
  ERROR erreurs qui pourraient encore permettre à l'application de continuer à s'exécuter.
  FATAL erreurs très graves qui conduiront probablement à l'arrêt de l'application.
  OFF   Désactive la journalisation.

Lorsque la sortie se produit sur la console :
  TRACE, DEBUG, INFO sortie sur cout
  WARN, ERROR, FATAL sortie sur cerr

Variables d'environnement ou options :
(Les options on la priorité sur les variables d'environnement)
  LOG est utilisé pour spécifier le type de journalisation ou pas de journalisation si définie sur OFF ou indéfinie.
  LOGFILE est utilisé pour spécifier le fichier qui pourrait recevoir les messages de journalisation. Par défaut la journalisation s'affiche sur la console, s'il y en a une. Sous windows on test si l'application s'execute en mode CUI (console) ou GUI
(graphique) et dans ce dernier cas on envoie aussi les logs vers un fichier du style "[app_name].log".

Si LOG n'est pas défini alors que LOGFILE l'est, le niveau de journalisation par défaut est défini sur ALL. Chaque message de journalisation commence par un horodatage au format ISO8601 et son niveau de journalisation au format suivant:
"yyyy-mm-ddThh:MM:ss+/-TZ:LOGLEVEL:Message"
*/

std::vector<std::pair<std::string, eLogLevel>> vLogLev = {
    {"ALL", LEVEL_ALL}, {"TRACE", LEVEL_TRACE}, {"DEBUG", LEVEL_DEBUG}, {"INFO", LEVEL_INFO}, {"WARN", LEVEL_WARN}, {"ERROR", LEVEL_ERROR}, {"FATAL", LEVEL_FATAL}, {"OFF", LEVEL_OFF},
};

std::string get_sloglev(eLogLevel ll)
{
  for (auto e : vLogLev)
  {
    if (ll == e.second)
      return e.first;
  }

  return "Undefined log level";
}

eLogLevel get_eloglev(std::string sll)
{
  sll = to_upper(sll);

  for (auto e : vLogLev)
  {
    if (sll == e.first)
      return e.second;
  }

  return LEVEL_UNDEFINED;
}

// This function must not be call within send2log
// It may used elsewhere to check and eventually correct the log level environment variable
void check_log_level()
{
  std::string sll = my_getenv("LOG");
  if (get_eloglev(sll) == LEVEL_UNDEFINED)
  {
    my_setenv("LOG", "WARN");
    logWarn("Unknown log level '" + sll + "', defaulting to 'WARN'");
  }
}

const std::string LOG_SEP = ";";
// const std::string CR_REP = "##<CR>##";
const std::string CR_REP = " ";

void send2log(eLogLevel llvl, std::string fn, int ln, std::string msg)
{
  eLogLevel eEnvLog;

  std::string envLog = my_getenv("LOG");
  std::string logfile = my_getenv("LOGFILE");

  if (envLog.empty())
  {
    if (logfile.empty())
      eEnvLog = LEVEL_OFF;
    else
      eEnvLog = LEVEL_ALL;
  }
  else
  {
    eEnvLog = get_eloglev(envLog);
    if (eEnvLog == LEVEL_UNDEFINED)
    { // Silently set undefined log level to warn log level
      eEnvLog = LEVEL_WARN;
    }
  }

  if (!logfile.empty() && eEnvLog == LEVEL_OFF)
    eEnvLog = LEVEL_ALL;

  if (eEnvLog == LEVEL_OFF || llvl < eEnvLog)
    return;

  // Nettoyage du message de log
  trim(msg);
  rtrim(msg, "\n");
  trim(msg);

  std::stringstream log_s;
#ifndef MULTI_ONE
  // Si message multi-ligne on déporte toutes les lignes à partir de la 2éme vers la derniere colonne de droite
  replace_all(msg, "\n", CR_REP);
  log_s << ISO8601() << LOG_SEP << get_sloglev((eLogLevel)llvl) << LOG_SEP << std::filesystem::path(fn).filename().string() << LOG_SEP << std::dec << ln << LOG_SEP << msg << std::endl << std::flush;
#else
  auto vms = split(msg, "\n");

  for (auto ms : vms)
  {
    log_s << ISO8601() << LOG_SEP << get_sloglev((eLogLevel)llvl) << LOG_SEP << std::filesystem::path(fn).filename().string() << LOG_SEP << std::dec << ln << LOG_SEP << ms << std::endl << std::flush;
  }
#endif

  std::string logpath = {};
  if (!logfile.empty() && logfile != "console")
  {
    logpath = logfile;
  }
  else
  {
#ifdef _WIN32
    logpath = my_getenv("TEMP") + "/webapp.log";
#else
    logpath = "/tmp/webapp.log";
#endif
  }

  std::ofstream log_f;
  log_f.open(logpath, std::ios_base::app);

  if (log_f.is_open())
  {
    log_f << log_s.str() << std::flush;
    log_f.close();
  }

  if (logfile == "console")
  {
    if (llvl < LEVEL_WARN)
      std::cout << log_s.str() << std::flush;
    else
      std::cerr << log_s.str() << std::flush;
  }
}
