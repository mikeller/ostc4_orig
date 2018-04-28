///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/text_multilanguage.c
/// \brief  Date file of TXT Multilanguage Support
/// \author heinrichs weikamp gmbh
/// \date   20-April-2014
///
/// \details
///     All displayed texts, in ENglish, DEutch, FRench, ITalian and ESpagnol.
///     When a translation is left empty, the English one is used instead.
///     Those texts are expanded from one-byte or two-bytes special code.
///
/// $Id$
///////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) 2014-2018 Heinrichs Weikamp gmbh
///
///     This program is free software: you can redistribute it and/or modify
///     it under the terms of the GNU General Public License as published by
///     the Free Software Foundation, either version 3 of the License, or
///     (at your option) any later version.
///
///     This program is distributed in the hope that it will be useful,
///     but WITHOUT ANY WARRANTY; without even the implied warranty of
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///     GNU General Public License for more details.
///
///     You should have received a copy of the GNU General Public License
///     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "text_multilanguage.h"

/* Text ----------------------------------------------------------------------*/

// Menu
static uint8_t text_EN_Language[] = "Language";
static uint8_t text_DE_Language[] = "Sprache";
static uint8_t text_FR_Language[] = "Langue";
static uint8_t text_IT_Language[] = "Lingua";
static uint8_t text_ES_Language[] = "Idioma";

// Menu
static uint8_t text_EN_LanguageName[] = "English";
static uint8_t text_DE_LanguageName[] = "Deutsch";
static uint8_t text_FR_LanguageName[] = "Français";
static uint8_t text_IT_LanguageName[] = "Italiano";
static uint8_t text_ES_LanguageName[] = "Castellano";

// dive mode
static uint8_t text_EN_Depth[] = "Depth";
static uint8_t text_DE_Depth[] = "Tiefe";
static uint8_t text_FR_Depth[] = "Profondeur";
static uint8_t text_IT_Depth[] = "Profondit�";
static uint8_t text_ES_Depth[] = "";

// dive mode
static uint8_t text_EN_Divetime[] = "Divetime";
static uint8_t text_DE_Divetime[] = "Tauchzeit";
static uint8_t text_FR_Divetime[] = "Durée";
static uint8_t text_IT_Divetime[] = "Tempo di Imm.";
static uint8_t text_ES_Divetime[] = "";

// dive mode
static uint8_t text_EN_MaxDepth[] = "Max Depth";
static uint8_t text_DE_MaxDepth[] = "Max Tiefe";
static uint8_t text_FR_MaxDepth[] = "Prof. max";    // Profondeur
static uint8_t text_IT_MaxDepth[] = "Profondit� Max";
static uint8_t text_ES_MaxDepth[] = "";

// dive mode
static uint8_t text_EN_AvgDepth[] = "Avg Depth";
static uint8_t text_DE_AvgDepth[] = "(/) Tiefe";
static uint8_t text_FR_AvgDepth[] = "Prof. Moy";
static uint8_t text_IT_AvgDepth[] = "Profondit� media";
static uint8_t text_ES_AvgDepth[] = "";

// dive mode
static uint8_t text_EN_Decostop[] = "Decostop";
static uint8_t text_DE_Decostop[] = "Dekostopp";
static uint8_t text_FR_Decostop[] = "Palier"; // Déco
static uint8_t text_IT_Decostop[] = "DecoStop";
static uint8_t text_ES_Decostop[] = "";

// dive mode
static uint8_t text_EN_Nullzeit[] = "NDL";
static uint8_t text_DE_Nullzeit[] = "Nullzeit";
static uint8_t text_FR_Nullzeit[] = "NoDéco";       // Sans palier
static uint8_t text_IT_Nullzeit[] = "No Deco";
static uint8_t text_ES_Nullzeit[] = "";

// dive mode
static uint8_t text_EN_ppO2[] = "ppO2";
static uint8_t text_DE_ppO2[] = "";
static uint8_t text_FR_ppO2[] = "PpO2";
static uint8_t text_IT_ppO2[] = "PpO2";
static uint8_t text_ES_ppO2[] = "";

// dive mode
static uint8_t text_EN_TTS[] = "TTS";
static uint8_t text_DE_TTS[] = "";
static uint8_t text_FR_TTS[] = "DTR";
static uint8_t text_IT_TTS[] = "TTS";
static uint8_t text_ES_TTS[] = "";

// dive mode
static uint8_t text_EN_CNSshort[] = "CNS";
static uint8_t text_DE_CNSshort[] = "";
static uint8_t text_FR_CNSshort[] = "SNC";
static uint8_t text_IT_CNSshort[] = "CNS";
static uint8_t text_ES_CNSshort[] = "";

// dive mode
static uint8_t text_EN_Temperature[] = "Temp.";
static uint8_t text_DE_Temperature[] = "Temperatur";
static uint8_t text_FR_Temperature[] = ""; // Température (ist zu lang)
static uint8_t text_IT_Temperature[] = "Temperatura";
static uint8_t text_ES_Temperature[] = "";

// dive mode
static uint8_t text_EN_FutureTTS[] = "FutureTTS";
static uint8_t text_DE_FutureTTS[] = "";
static uint8_t text_FR_FutureTTS[] = "FutureDTR";
static uint8_t text_IT_FutureTTS[] = "TTS Futuro";
static uint8_t text_ES_FutureTTS[] = "";

// Menu (as small text)
static uint8_t text_EN_Minutes[] = "minutes";
static uint8_t text_DE_Minutes[] = "Minuten";
static uint8_t text_FR_Minutes[] = "minutes";
static uint8_t text_IT_Minutes[] = "Minuti";
static uint8_t text_ES_Minutes[] = "";

// Menu SYS2 sub (as small text in custom views)
static uint8_t text_EN_Seconds[] = "Seconds";
static uint8_t text_DE_Seconds[] = "Sekunden";
static uint8_t text_FR_Seconds[] = "Secondes";
static uint8_t text_IT_Seconds[] = "Secondi";
static uint8_t text_ES_Seconds[] = "";

// Menu OC sub and Menu CC sub
static uint8_t text_EN_Gas[] = "Gas";
static uint8_t text_DE_Gas[] = "";
static uint8_t text_FR_Gas[] = "Gaz";
static uint8_t text_IT_Gas[] = "Gas";
static uint8_t text_ES_Gas[] = "";

// dive mode
static uint8_t text_EN_Ceiling[] = "Ceiling";
static uint8_t text_DE_Ceiling[] = "";
static uint8_t text_FR_Ceiling[] = "Plafond";
static uint8_t text_IT_Ceiling[] = "Ceiling";
static uint8_t text_ES_Ceiling[] = "";

// dive mode
static uint8_t text_EN_ActualGradient[] = "Relative GF";
static uint8_t text_DE_ActualGradient[] = "";
static uint8_t text_FR_ActualGradient[] = "GF Relatif";
static uint8_t text_IT_ActualGradient[] = "GF";
static uint8_t text_ES_ActualGradient[] = "";

// dive mode
static uint8_t text_EN_Stopwatch[] = "Stopwatch";
static uint8_t text_DE_Stopwatch[] = "Stoppuhr";
static uint8_t text_FR_Stopwatch[] = "Chrono";
static uint8_t text_IT_Stopwatch[] = "Stop Cronometro";
static uint8_t text_ES_Stopwatch[] = "";

// Menu SYS1 sub
static uint8_t text_EN_CompassCalib[] = "Calibrate compass";
static uint8_t text_DE_CompassCalib[] = "Kompass kalibrieren";
static uint8_t text_FR_CompassCalib[] = "Calibration boussole";
static uint8_t text_IT_CompassCalib[] = "Calibrazione bussola";
static uint8_t text_ES_CompassCalib[] = "";

// Menu SYS1 and Customview header
static uint8_t text_EN_Compass[] = "Compass";
static uint8_t text_DE_Compass[] = "Kompass";
static uint8_t text_FR_Compass[] = "Boussole";
static uint8_t text_IT_Compass[] = "Bussola";
static uint8_t text_ES_Compass[] = "";

// Menu SYS1
static uint8_t text_EN_o2Sensors[] = "oxygen sensors";
static uint8_t text_DE_o2Sensors[] = "O2 Sensoren";
static uint8_t text_FR_o2Sensors[] = "Cellules O2";
static uint8_t text_IT_o2Sensors[] = "Sensore O2";
static uint8_t text_ES_o2Sensors[] = "";

// Menu SYS1
static uint8_t text_EN_Brightness[] = "Brightness";
static uint8_t text_DE_Brightness[] = "Display Helligkeit";
static uint8_t text_FR_Brightness[] = "Luminosité";
static uint8_t text_IT_Brightness[] = "Luminosit�";
static uint8_t text_ES_Brightness[] = "";

// Menu SYS1
static uint8_t text_EN_Cave[] = "Cave";
static uint8_t text_DE_Cave[] = "Eco+";
static uint8_t text_FR_Cave[] = "Mode éco+";
static uint8_t text_IT_Cave[] = "Grotta";
static uint8_t text_ES_Cave[] = "";

// Menu SYS1
static uint8_t text_EN_Eco[] = "Eco";
static uint8_t text_DE_Eco[] = "Eco-Sparmodus";
static uint8_t text_FR_Eco[] = "Mode éco";
static uint8_t text_IT_Eco[] = "Modalit� ECO";
static uint8_t text_ES_Eco[] = "";

// Menu SYS1
static uint8_t text_EN_Normal[] = "Medium";
static uint8_t text_DE_Normal[] = "Standard";
static uint8_t text_FR_Normal[] = "Moyenne";
static uint8_t text_IT_Normal[] = "Medio";
static uint8_t text_ES_Normal[] = "";

// Menu SYS1
static uint8_t text_EN_Bright[] = "High";
static uint8_t text_DE_Bright[] = "Hoch";
static uint8_t text_FR_Bright[] = "Haute";
static uint8_t text_IT_Bright[] = "Alto";
static uint8_t text_ES_Bright[] = "";

// Menu SYS1
static uint8_t text_EN_Ultrabright[] = "Ultra Bright";
static uint8_t text_DE_Ultrabright[] = "Max";
static uint8_t text_FR_Ultrabright[] = "Mode max";
static uint8_t text_IT_Ultrabright[] = "Max";
static uint8_t text_ES_Ultrabright[] = "";

// Menu OC (Header)
static uint8_t text_EN_OC_Gas_Edit[] = "Open Circuit";
static uint8_t text_DE_OC_Gas_Edit[] = "";
static uint8_t text_FR_OC_Gas_Edit[] = "Circuit ouvert";
static uint8_t text_IT_OC_Gas_Edit[] = "Circuito Aperto";
static uint8_t text_ES_OC_Gas_Edit[] = "";

// Menu CC (Header)
static uint8_t text_EN_Diluent_Gas_Edit[] = "Diluent";
static uint8_t text_DE_Diluent_Gas_Edit[] = "";
static uint8_t text_FR_Diluent_Gas_Edit[] = "Diluant";
static uint8_t text_IT_Diluent_Gas_Edit[] = "Diluente";
static uint8_t text_ES_Diluent_Gas_Edit[] = "";

// Menu Gas
static uint8_t text_EN_Mix[] = "Mix";
static uint8_t text_DE_Mix[] = "";
static uint8_t text_FR_Mix[] = "Gaz";  // Or Mélange
static uint8_t text_IT_Mix[] = "Gas MIX";
static uint8_t text_ES_Mix[] = "";

// Menu Gas
static uint8_t text_EN_Active[] = "Active";
static uint8_t text_DE_Active[] = "Aktiv";
static uint8_t text_FR_Active[] = "Activé";
static uint8_t text_IT_Active[] = "Attivo";
static uint8_t text_ES_Active[] = "";

// Menu Gas
static uint8_t text_EN_First[] = "First";
static uint8_t text_DE_First[] = "Start";
static uint8_t text_FR_First[] = "Premier";
static uint8_t text_IT_First[] = "Primo";
static uint8_t text_ES_First[] = "";

// Menu Gas
static uint8_t text_EN_Deco[] = "Deco";
static uint8_t text_DE_Deco[] = "Deko";
static uint8_t text_FR_Deco[] = "Déco";
static uint8_t text_IT_Deco[] = "Deco";
static uint8_t text_ES_Deco[] = "";

// Menu Gas
static uint8_t text_EN_Travel[] = "Travel";
static uint8_t text_DE_Travel[] = "Reise";
static uint8_t text_FR_Travel[] = ""; // We always use english here I think...
static uint8_t text_IT_Travel[] = "Viaggio";
static uint8_t text_ES_Travel[] = "";

// Menu Gas
static uint8_t text_EN_Inactive[] = "Inactive";
static uint8_t text_DE_Inactive[] = "Deaktiviert";
static uint8_t text_FR_Inactive[] = "Desactivé";
static uint8_t text_IT_Inactive[] = "Disattivato";
static uint8_t text_ES_Inactive[] = "";

// Menu Gas
static uint8_t text_EN_ChangeDepth[] = "ChangeDepth";
static uint8_t text_DE_ChangeDepth[] = "Wechseltiefe";
static uint8_t text_FR_ChangeDepth[] = "Prof.Changmt"; // "Profondeur de changement";  // Prof.Changmt
static uint8_t text_IT_ChangeDepth[] = "Cambio Prof.";
static uint8_t text_ES_ChangeDepth[] = "";

// Menu Gas
static uint8_t text_EN_Type[] = "Type";
static uint8_t text_DE_Type[] = "Typ";
static uint8_t text_FR_Type[] = "";
static uint8_t text_IT_Type[] = "Tipo";
static uint8_t text_ES_Type[] = "";

// Menu SP (Part of Header)
static uint8_t text_EN_Setpoint_Edit[] = "Setup";
static uint8_t text_DE_Setpoint_Edit[] = "Einstellung";
static uint8_t text_FR_Setpoint_Edit[] = "Configuration";
static uint8_t text_IT_Setpoint_Edit[] = "Configurazione";
static uint8_t text_ES_Setpoint_Edit[] = "";

// Menu SYS1 (O2 Sensors)
static uint8_t text_EN_Fallback[] = "Fallback";
static uint8_t text_DE_Fallback[] = "";
static uint8_t text_FR_Fallback[] = "Secours";      // Or "Repli"
static uint8_t text_IT_Fallback[] = "Ricalibrare";
static uint8_t text_ES_Fallback[] = "";

// Menu DECO2
static uint8_t text_EN_Algorithm[] = "Algorithm";
static uint8_t text_DE_Algorithm[] = "Rechenmodell";
static uint8_t text_FR_Algorithm[] = ""; // Algorithm
static uint8_t text_IT_Algorithm[] = "Algoritmo";
static uint8_t text_ES_Algorithm[] = "";

// Menu DECO1
static uint8_t text_EN_SafetyStop[] = "Safety stop";
static uint8_t text_DE_SafetyStop[] = "Sicherheitsstop";
static uint8_t text_FR_SafetyStop[] = "Palier sécurité";
static uint8_t text_IT_SafetyStop[] = "Tappa di sicurezza";
static uint8_t text_ES_SafetyStop[] = "";

// Menu DECO1 (CCR mode only)
static uint8_t text_EN_CCRmode[] = "CCR mode";
static uint8_t text_DE_CCRmode[] = "CCR Modus";
static uint8_t text_FR_CCRmode[] = "Mode CCR";
static uint8_t text_IT_CCRmode[] = "CCR mode";
static uint8_t text_ES_CCRmode[] = "";

// Menu DECO1 (CCR mode only)
static uint8_t text_EN_Sensor[] = "Sensor";
static uint8_t text_DE_Sensor[] = "";
static uint8_t text_FR_Sensor[] = "Cellule";
static uint8_t text_IT_Sensor[] = "Sensore";
static uint8_t text_ES_Sensor[] = "";

// Menu DECO1 (CCR mode only)
static uint8_t text_EN_FixedSP[] = "Fixed SP";
static uint8_t text_DE_FixedSP[] = "Fester SP";
static uint8_t text_FR_FixedSP[] = "SP Fixe";
static uint8_t text_IT_FixedSP[] = "Set Point Fisso";
static uint8_t text_ES_FixedSP[] = "";

// Menu DECO2 (VPM and Buehlmann Sub Menu)
static uint8_t text_EN_Decoparameters[] = "Deco Parameters";
static uint8_t text_DE_Decoparameters[] = "Deko Parameter";
static uint8_t text_FR_Decoparameters[] = "Paramètres déco";
static uint8_t text_IT_Decoparameters[] = "Parametri Deco";
static uint8_t text_ES_Decoparameters[] = "";

// Menu DECO2
static uint8_t text_EN_LastDecostop[] = "Last Deco";
static uint8_t text_DE_LastDecostop[] = "Letzter Stop";
static uint8_t text_FR_LastDecostop[] = "Dern. palier";
static uint8_t text_IT_LastDecostop[] = "Ultima Deco";
static uint8_t text_ES_LastDecostop[] = "";

// Menu DECO2 and Dive Menu
static uint8_t text_EN_ZHL16GF[] = "ZH-L16+GF";
static uint8_t text_DE_ZHL16GF[] = "";
static uint8_t text_FR_ZHL16GF[] = "";
static uint8_t text_IT_ZHL16GF[] = "";
static uint8_t text_ES_ZHL16GF[] = "";

// Menu DECO2 and Dive Menu
static uint8_t text_EN_aGF[] = "aGF";
static uint8_t text_DE_aGF[] = "";
static uint8_t text_FR_aGF[] = "";
static uint8_t text_IT_aGF[] = "GF Alternativo";
static uint8_t text_ES_aGF[] = "";

// Menu DECO2 and Dive Menu
static uint8_t text_EN_VPM[] = "VPM";
static uint8_t text_DE_VPM[] = "";
static uint8_t text_FR_VPM[] = "";
static uint8_t text_IT_VPM[] = "";
static uint8_t text_ES_VPM[] = "";

// Dive Menu
static uint8_t text_EN_LowHigh[] = "low         high";
static uint8_t text_DE_LowHigh[] = "";
static uint8_t text_FR_LowHigh[] = "Bas         Haut";
static uint8_t text_IT_LowHigh[] = "Basso       Alto";
static uint8_t text_ES_LowHigh[] = "";

// Menu DECO1 Sub
static uint8_t text_EN_ppO2Name[] = "Partial Pressure Oxygen";
static uint8_t text_DE_ppO2Name[] = "Sauerstoff-Partialdruck";
static uint8_t text_FR_ppO2Name[] = "Pression partl. oxygène";
static uint8_t text_IT_ppO2Name[] = "Pressione parziale Ossigeno";
static uint8_t text_ES_ppO2Name[] = "";

static uint8_t text_EN_Maximum[] = "Maximum";
static uint8_t text_DE_Maximum[] = "";
static uint8_t text_FR_Maximum[] = "";
static uint8_t text_IT_Maximum[] = "Massimo";
static uint8_t text_ES_Maximum[] = "";

static uint8_t text_EN_Minimum[] = "Minimum";
static uint8_t text_DE_Minimum[] = "";
static uint8_t text_FR_Minimum[] = "";
static uint8_t text_IT_Minimum[] = "Minimo";
static uint8_t text_ES_Minimum[] = "";

static uint8_t text_EN_Salinity[] = "Salinity";
static uint8_t text_DE_Salinity[] = "Salzgehalt";
static uint8_t text_FR_Salinity[] = "Salinité";
static uint8_t text_IT_Salinity[] = "Salinit�";
static uint8_t text_ES_Salinity[] = "";

// Menu DECO1
static uint8_t text_EN_DiveMode[] = "Dive Mode";
static uint8_t text_DE_DiveMode[] = "Tauchmodus";
static uint8_t text_FR_DiveMode[] = "Mode Plongée";
static uint8_t text_IT_DiveMode[] = "Dive Mode";
static uint8_t text_ES_DiveMode[] = "";

// Menu DECO1
static uint8_t text_EN_OpenCircuit[] = "Open Circuit";
static uint8_t text_DE_OpenCircuit[] = "offener Kreislauf";
static uint8_t text_FR_OpenCircuit[] = "Circuit ouvert";
static uint8_t text_IT_OpenCircuit[] = "Circuito Aperto";
static uint8_t text_ES_OpenCircuit[] = "";

// Menu DECO1
static uint8_t text_EN_ClosedCircuit[] = "Closed Circuit";
static uint8_t text_DE_ClosedCircuit[] = "geschlossen/CCR";
static uint8_t text_FR_ClosedCircuit[] = "Recycleur";
static uint8_t text_IT_ClosedCircuit[] = "Ciurcuito Chiuso";
static uint8_t text_ES_ClosedCircuit[] = "";

static uint8_t text_EN_Time[] = "Time";
static uint8_t text_DE_Time[] = "Uhrzeit";
static uint8_t text_FR_Time[] = "Heure";
static uint8_t text_IT_Time[] = "Tempo";
static uint8_t text_ES_Time[] = "";

static uint8_t text_EN_Date[] = "Date";
static uint8_t text_DE_Date[] = "Datum";
static uint8_t text_FR_Date[] = ""; // Date
static uint8_t text_IT_Date[] = "Data";
static uint8_t text_ES_Date[] = "";

static uint8_t text_EN_Format[] = "Format";
static uint8_t text_DE_Format[] = "Format";
static uint8_t text_FR_Format[] = ""; // Format
static uint8_t text_IT_Format[] = "Formato";
static uint8_t text_ES_Format[] = "";

// Menu SYS2 sub
static uint8_t text_EN_DateTime[] = "Date and time";
static uint8_t text_DE_DateTime[] = "Datum und Uhrzeit";
static uint8_t text_FR_DateTime[] = "Date et heure";
static uint8_t text_IT_DateTime[] = "Data e ora";
static uint8_t text_ES_DateTime[] = "";

static uint8_t text_EN_DayMonthYear[] = "Date";
static uint8_t text_DE_DayMonthYear[] = "Datum";
static uint8_t text_FR_DayMonthYear[] = ""; // Date
static uint8_t text_IT_DayMonthYear[] = "Data";
static uint8_t text_ES_DayMonthYear[] = "";

static uint8_t text_EN_StundeMinute[] = "Time";
static uint8_t text_DE_StundeMinute[] = "Uhrzeit";
static uint8_t text_FR_StundeMinute[] = "Heure";
static uint8_t text_IT_StundeMinute[] = "Time";
static uint8_t text_ES_StundeMinute[] = "";

// Logbook
static uint8_t text_EN_Logbook[] = "Logbook";
static uint8_t text_DE_Logbook[] = "Logbuch";
static uint8_t text_FR_Logbook[] = "Carnet";
static uint8_t text_IT_Logbook[] = "Logbook";
static uint8_t text_ES_Logbook[] = "";

// Logbook
static uint8_t text_EN_LogbookEmpty[] = "Logbook empty.";
static uint8_t text_DE_LogbookEmpty[] = "Logbuch leer.";
static uint8_t text_FR_LogbookEmpty[] = "Carnet vide";
static uint8_t text_IT_LogbookEmpty[] = "Logbook vuoto";
static uint8_t text_ES_LogbookEmpty[] = "";

// Menu SIM
static uint8_t text_EN_Start_Calculation[] = "Start Calculation";
static uint8_t text_DE_Start_Calculation[] = "";
static uint8_t text_FR_Start_Calculation[] = "Calculer";
static uint8_t text_IT_Start_Calculation[] = "Calcola";
static uint8_t text_ES_Start_Calculation[] = "";

// Menu SYS2
static uint8_t text_EN_Design[] = "Layout";
static uint8_t text_DE_Design[] = "";
static uint8_t text_FR_Design[] = "Affichage";
static uint8_t text_IT_Design[] = "Formato schermo";
static uint8_t text_ES_Design[] = "";

// Menu SYS2
static uint8_t text_EN_Farbschema[] = "Color scheme";
static uint8_t text_DE_Farbschema[] = "Farbschema";
static uint8_t text_FR_Farbschema[] = "Jeu de Couleurs";
static uint8_t text_IT_Farbschema[] = "Colore schermo";
static uint8_t text_ES_Farbschema[] = "";

// Menu SYS2
static uint8_t text_EN_Customviews[] = "Custom Views";
static uint8_t text_DE_Customviews[] = "Variable Anzeigen";
static uint8_t text_FR_Customviews[] = "Affichage personnel";//"Affich. perso.";
static uint8_t text_IT_Customviews[] = "Personalizza Schermo";
static uint8_t text_ES_Customviews[] = "";

// Menu SYS2 sub
static uint8_t text_EN_CViewTimeout[] = "Center auto return after";
static uint8_t text_DE_CViewTimeout[] = "Mitte automat. zurück";
static uint8_t text_FR_CViewTimeout[] = "Retour affich. central";
static uint8_t text_IT_CViewTimeout[] = "Centro Auto Ritorno";
static uint8_t text_ES_CViewTimeout[] = "";

// Menu SYS2 sub
static uint8_t text_EN_CViewStandard[] = "Center primary";
static uint8_t text_DE_CViewStandard[] = "Mitte primär";
static uint8_t text_FR_CViewStandard[] = "Affichage central";
static uint8_t text_IT_CViewStandard[] = "Display Centrale";
static uint8_t text_ES_CViewStandard[] = "";

// Menu SYS2 sub
static uint8_t text_EN_CornerTimeout[] = "Field auto return after";
static uint8_t text_DE_CornerTimeout[] = "Feld automat. zurück";
static uint8_t text_FR_CornerTimeout[] = "Retour affich. gauche";     // This is the LEFT corner.
static uint8_t text_IT_CornerTimeout[] = "Campo Auto Ritorno";
static uint8_t text_ES_CornerTimeout[] = "";

// Menu SYS2 sub
static uint8_t text_EN_CornerStandard[] = "Field primary";
static uint8_t text_DE_CornerStandard[] = "Feld primär";
static uint8_t text_FR_CornerStandard[] = "Affichage gauche";
static uint8_t text_IT_CornerStandard[] = "Campo Primario";
static uint8_t text_ES_CornerStandard[] = "";

// Menu GAS sub
static uint8_t text_EN_SetToMOD[] = "Set Change Depth To MOD";
static uint8_t text_DE_SetToMOD[] = "Setze Wechseltiefe auf MOD";
static uint8_t text_FR_SetToMOD[] = "Régler prof. chgmt. à PMU";
static uint8_t text_IT_SetToMOD[] = "Cambia profondit�";
static uint8_t text_ES_SetToMOD[] = "";

// Menu SYS2 sub
static uint8_t text_EN_Units[] = "Units";
static uint8_t text_DE_Units[] = "Einheit";
static uint8_t text_FR_Units[] = "Unités";
static uint8_t text_IT_Units[] = "Unit�";
static uint8_t text_ES_Units[] = "";

// Menu SYS2 sub
static uint8_t text_EN_Design_t7_feet[] = "feet/fahrenheit";
static uint8_t text_DE_Design_t7_feet[] = "";
static uint8_t text_FR_Design_t7_feet[] = "pied/Fahrenheit";
static uint8_t text_IT_Design_t7_feet[] = "Piedi/Fahrenheit";
static uint8_t text_ES_Design_t7_feet[] = "";

// Menu SYS2 sub
static uint8_t text_EN_Design_t7_metric[] = "metric system";
static uint8_t text_DE_Design_t7_metric[] = "meter/Celsius";
static uint8_t text_FR_Design_t7_metric[] = "mètre/Celsius";
static uint8_t text_IT_Design_t7_metric[] = "Sistema Metrico";
static uint8_t text_ES_Design_t7_metric[] = "";

// Menu SYS2
static uint8_t text_EN_Information[] = "Information";
static uint8_t text_DE_Information[] = "";
static uint8_t text_FR_Information[] = ""; // Information
static uint8_t text_IT_Information[] = "Info";
static uint8_t text_ES_Information[] = "";

// Menu SYS2
static uint8_t text_EN_ResetMenu[] = "Reset Menu";
static uint8_t text_DE_ResetMenu[] = "Reset Menü";
static uint8_t text_FR_ResetMenu[] = "Menu RaZ"; // RàZ
static uint8_t text_IT_ResetMenu[] = "Ripristina Menu";
static uint8_t text_ES_ResetMenu[] = "";

// Menu SYS2 sub
static uint8_t text_EN_LogbookOffset[] = "Logbook offset";
static uint8_t text_DE_LogbookOffset[] = "Logbuch Versatz";
static uint8_t text_FR_LogbookOffset[] = "Num. 1er plongée";
static uint8_t text_IT_LogbookOffset[] = "Loogbook offset";
static uint8_t text_ES_LogbookOffset[] = "";


// Menu SYS2 sub
static uint8_t text_EN_Maintenance[] = "Maintenance";
static uint8_t text_DE_Maintenance[] = "";
static uint8_t text_FR_Maintenance[] = "";
static uint8_t text_IT_Maintenance[] = "Manutenzione";
static uint8_t text_ES_Maintenance[] = "";

// Menu SYS2 sub
static uint8_t text_EN_SetBatteryCharge[] = "Restore Battery Charge";
static uint8_t text_DE_SetBatteryCharge[] = "";
static uint8_t text_FR_SetBatteryCharge[] = "";
static uint8_t text_IT_SetBatteryCharge[] = "Ricaricare batteria";
static uint8_t text_ES_SetBatteryCharge[] = "";

// Menu SYS2 sub
static uint8_t text_EN_SetFactoryDefaults[] = "Store Button Factory Defaults";
static uint8_t text_DE_SetFactoryDefaults[] = "";
static uint8_t text_FR_SetFactoryDefaults[] = "";
static uint8_t text_IT_SetFactoryDefaults[] = "Impostazioni di fabbrica";
static uint8_t text_ES_SetFactoryDefaults[] = "";

// Menu SYS2 sub
static uint8_t text_EN_Reboot[] = "Reboot";
static uint8_t text_DE_Reboot[] = "Neustart";
static uint8_t text_FR_Reboot[] = "Redémarrage";
static uint8_t text_IT_Reboot[] = "Riavvio";
static uint8_t text_ES_Reboot[] = "";

// Menu SYS2 sub
static uint8_t text_EN__RebootRTE[] = "Reboot RTE";
static uint8_t text_DE__RebootRTE[] = "RTE neu starten";
static uint8_t text_FR__RebootRTE[] = "Redémarrage RTE";
static uint8_t text_IT__RebootRTE[] = "Riavvio RTE";
static uint8_t text_ES__RebootRTE[] = "";

// Menu SYS2 sub
static uint8_t text_EN_AreYouSure[] = "Are you sure?";
static uint8_t text_DE_AreYouSure[] = "Sind Sie sicher?";
static uint8_t text_FR_AreYouSure[] = "Confirmer?";
static uint8_t text_IT_AreYouSure[] = "Sei sicuro?";
static uint8_t text_ES_AreYouSure[] = "";

// Menu SYS2 sub
static uint8_t text_EN_Abort[] = "Abort";
static uint8_t text_DE_Abort[] = "Abbrechen";
static uint8_t text_FR_Abort[] = "Quitter";
static uint8_t text_IT_Abort[] = "Uscita";
static uint8_t text_ES_Abort[] = "";

// Menu SYS2 sub
static uint8_t text_EN_ResetAll[] = "Reset Settings";
static uint8_t text_DE_ResetAll[] = "Einstellung zurücksetzen";
static uint8_t text_FR_ResetAll[] = "RaZ Paramètres";
static uint8_t text_IT_ResetAll[] = "Ripristina Impostazioni";
static uint8_t text_ES_ResetAll[] = "";

// Menu SYS2 sub
static uint8_t text_EN_ResetDeco[] = "Reset Deco";
static uint8_t text_DE_ResetDeco[] = "Deko zurücksetzen";
static uint8_t text_FR_ResetDeco[] = "RaZ Deco";
static uint8_t text_IT_ResetDeco[] = "Ripristina Deco";
static uint8_t text_ES_ResetDeco[] = "";

// Menu SYS2 sub
static uint8_t text_EN_Exit[] = "Exit";
static uint8_t text_DE_Exit[] = "Ende";
static uint8_t text_FR_Exit[] = "Sortir";
static uint8_t text_IT_Exit[] = "Esci";
static uint8_t text_ES_Exit[] = "";

// Menu SYS2 sub
static uint8_t text_EN_StartBootloader[] = "Reboot Firmware";
static uint8_t text_DE_StartBootloader[] = "Neustart Firmware";
static uint8_t text_FR_StartBootloader[] = "Redémarrer";
static uint8_t text_IT_StartBootloader[] = "Ripristina Firmware";
static uint8_t text_ES_StartBootloader[] = "";

// Menu SYS2 sub
static uint8_t text_EN_ResetLogbook[] = "Reset Logbook";
static uint8_t text_DE_ResetLogbook[] = "Logbuch zurücksetzen";
static uint8_t text_FR_ResetLogbook[] = "RaZ carnet"; // RàZ
static uint8_t text_IT_ResetLogbook[] = "Ripristina Logbook";
static uint8_t text_ES_ResetLogbook[] = "";

// Surface warning
static uint8_t text_EN_PleaseUpdate[] = "Please update";
static uint8_t text_DE_PleaseUpdate[] = "Bitte updaten:";
static uint8_t text_FR_PleaseUpdate[] = "Mettre à jours svp.";
static uint8_t text_IT_PleaseUpdate[] = "Aggiornamento";
static uint8_t text_ES_PleaseUpdate[] = "";

// Surface warning
static uint8_t text_EN_RTE[] = "RTE";
static uint8_t text_DE_RTE[] = "";
static uint8_t text_FR_RTE[] = "";
static uint8_t text_IT_RTE[] = "RTE";
static uint8_t text_ES_RTE[] = "";

// Surface warning
static uint8_t text_EN_Fonts[] = "fonts"; // Character fonts!!
static uint8_t text_DE_Fonts[] = "";
static uint8_t text_FR_Fonts[] = "polices";
static uint8_t text_IT_Fonts[] = "Carattere";
static uint8_t text_ES_Fonts[] = "";

// Dive Menu
static uint8_t text_EN_ResetStopwatch[] = "Reset stopwatch";
static uint8_t text_DE_ResetStopwatch[] = "Stoppuhr zurückstellen";
static uint8_t text_FR_ResetStopwatch[] = "RaZ chrono"; // RàZ
static uint8_t text_IT_ResetStopwatch[] = "Riavvia cronometro";
static uint8_t text_ES_ResetStopwatch[] = "";

// Dive Menu
static uint8_t text_EN_SetMarker[] = "Set marker";
static uint8_t text_DE_SetMarker[] = "Markierung";
static uint8_t text_FR_SetMarker[] = "Repère";
static uint8_t text_IT_SetMarker[] = "Coordinata";
static uint8_t text_ES_SetMarker[] = "";

// Dive Menu
static uint8_t text_EN_CompassHeading[] = "Compass heading";
static uint8_t text_DE_CompassHeading[] = "Kompasskurs";
static uint8_t text_FR_CompassHeading[] = "Cap";
static uint8_t text_IT_CompassHeading[] = "Direzione bussola";
static uint8_t text_ES_CompassHeading[] = "";

// Menu SIM
static uint8_t text_EN_Simulator[] = "Simulator";
static uint8_t text_DE_Simulator[] = "";
static uint8_t text_FR_Simulator[] = "Simulateur";
static uint8_t text_IT_Simulator[] = "Simulazione";
static uint8_t text_ES_Simulator[] = "";

// Menu SIM
static uint8_t text_EN_StartSimulator[] = "Start Simulator";
static uint8_t text_DE_StartSimulator[] = "Simulator starten";
static uint8_t text_FR_StartSimulator[] = "Démarrage Simulateur";
static uint8_t text_IT_StartSimulator[] = "Inizia Simulazione";
static uint8_t text_ES_StartSimulator[] = "";

// Menu SIM
static uint8_t text_EN_Intervall[] = "Interval";
static uint8_t text_DE_Intervall[] = "Intervall";
static uint8_t text_FR_Intervall[] = "Intervalle";
static uint8_t text_IT_Intervall[] = "Intervallo";
static uint8_t text_ES_Intervall[] = "";

// Menu SIM
static uint8_t text_EN_SimDiveTime[] = "Dive Time";
static uint8_t text_DE_SimDiveTime[] = "Tauchzeit";
static uint8_t text_FR_SimDiveTime[] = "Temps fond";
static uint8_t text_IT_SimDiveTime[] = "Tempo di Immersione";
static uint8_t text_ES_SimDiveTime[] = "";

// Menu SIM
static uint8_t text_EN_SimMaxDepth[] = "Max. Depth";
static uint8_t text_DE_SimMaxDepth[] = "Max. Tiefe";
static uint8_t text_FR_SimMaxDepth[] = "Prof. max";
static uint8_t text_IT_SimMaxDepth[] = "Profondit� Max.";
static uint8_t text_ES_SimMaxDepth[] = "";

// Menu SIM sub
static uint8_t text_EN_SimConsumption[] = "Gas consumption";
static uint8_t text_DE_SimConsumption[] = "Gas Verbrauch";
static uint8_t text_FR_SimConsumption[] = "Conso gaz";
static uint8_t text_IT_SimConsumption[] = "Consumo Gas";
static uint8_t text_ES_SimConsumption[] = "";

// Menu SIM sub
static uint8_t text_EN_SimSummary[] = "Summary";
static uint8_t text_DE_SimSummary[] = "Zusammenfassung";
static uint8_t text_FR_SimSummary[] = "Résumé";
static uint8_t text_IT_SimSummary[] = "Leggenda";
static uint8_t text_ES_SimSummary[] = "";

// Menu SIM sub
static uint8_t text_EN_SimDecTo[] = "Dec to";
static uint8_t text_DE_SimDecTo[] = "Abst.";
static uint8_t text_FR_SimDecTo[] = "Desc à";
static uint8_t text_IT_SimDecTo[] = "";
static uint8_t text_ES_SimDecTo[] = "";

// Menu SIM sub
static uint8_t text_EN_SimLevel[] = "Level";
static uint8_t text_DE_SimLevel[] = "Tiefe";
static uint8_t text_FR_SimLevel[] = "Niveau";
static uint8_t text_IT_SimLevel[] = "Livello";
static uint8_t text_ES_SimLevel[] = "";

// Menu SIM sub
static uint8_t text_EN_SimAscTo[] = "Asc to";
static uint8_t text_DE_SimAscTo[] = "Aufst.";
static uint8_t text_FR_SimAscTo[] = "Rem. à";
static uint8_t text_IT_SimAscTo[] = "";
static uint8_t text_ES_SimAscTo[] = "";

// Menu SIM sub
static uint8_t text_EN_SimSurface[] = "Surface";
static uint8_t text_DE_SimSurface[] = "Oberfl.";
static uint8_t text_FR_SimSurface[] = ""; // Surface
static uint8_t text_IT_SimSurface[] = "Superficie";
static uint8_t text_ES_SimSurface[] = "";

// Menu SIM sub
static uint8_t text_EN_Calculating[] = "Calculating ...";
static uint8_t text_DE_Calculating[] = "Auswertung ...";
static uint8_t text_FR_Calculating[] = "Calcul...";
static uint8_t text_IT_Calculating[] = "Elaborazione...";
static uint8_t text_ES_Calculating[] = "";

// Menu SIM sub
static uint8_t text_EN_PleaseWait[] = "Please wait!";
static uint8_t text_DE_PleaseWait[] = "Bitte warten!";
static uint8_t text_FR_PleaseWait[] = "Attendre svp.";
static uint8_t text_IT_PleaseWait[] = "Attendere...";
static uint8_t text_ES_PleaseWait[] = "";

// Menu SIM
static uint8_t text_EN_CalculateDeco[] = "Calculate Deco";
static uint8_t text_DE_CalculateDeco[] = "Deko berechnen";
static uint8_t text_FR_CalculateDeco[] = "Calcul Déco";
static uint8_t text_IT_CalculateDeco[] = "Calcolo Deco";
static uint8_t text_ES_CalculateDeco[] = "";

// Menu SIM sub
static uint8_t text_EN_Decolist[] = "Decoplan";
static uint8_t text_DE_Decolist[] = "Dekoplan";
static uint8_t text_FR_Decolist[] = "Runtime";
static uint8_t text_IT_Decolist[] = "Pianifica Deco";
static uint8_t text_ES_Decolist[] = "";

// Menu SYS1
static uint8_t text_EN_ButtonSensitivity[] = "Button sensitivity";
static uint8_t text_DE_ButtonSensitivity[] = "Taster Ansprechen";
static uint8_t text_FR_ButtonSensitivity[] = "Bouton sensibilité";
static uint8_t text_IT_ButtonSensitivity[] = "Sensibilit� Pulsante";
static uint8_t text_ES_ButtonSensitivity[] = "";

//
static uint8_t text_EN_SpecialDiveGas[] = "Free configurable";
static uint8_t text_DE_SpecialDiveGas[] = "Frei einstellbar";
static uint8_t text_FR_SpecialDiveGas[] = "";
static uint8_t text_IT_SpecialDiveGas[] = "Configurazione libera";
static uint8_t text_ES_SpecialDiveGas[] = "";

// Dive Menu
static uint8_t text_EN_SpecialDiveGasMenu[] = "Lost Gas and Extra Gas";
static uint8_t text_DE_SpecialDiveGasMenu[] = "Verlorene Gase und Extra Gas";
static uint8_t text_FR_SpecialDiveGasMenu[] = "";
static uint8_t text_IT_SpecialDiveGasMenu[] = "Gas perso e Extra Gas";
static uint8_t text_ES_SpecialDiveGasMenu[] = "";

// Dive Menu (CCR mode)
static uint8_t text_EN_SpecialDiveGasMenuCCR[] = "Lost Gas";
static uint8_t text_DE_SpecialDiveGasMenuCCR[] = "Verlorene Gase";
static uint8_t text_FR_SpecialDiveGasMenuCCR[] = "";
static uint8_t text_IT_SpecialDiveGasMenuCCR[] = "Gas perso";
static uint8_t text_ES_SpecialDiveGasMenuCCR[] = "";

// Dive Menu (CCR mode)
static uint8_t text_EN_UseSensor[] = "Use Sensor";
static uint8_t text_DE_UseSensor[] = "Benutze Sensor";
static uint8_t text_FR_UseSensor[] = "Cellules";
static uint8_t text_IT_UseSensor[] = "Sensore";
static uint8_t text_ES_UseSensor[] = "";

// Warning
static uint8_t text_EN_WarnDecoMissed[] = "deco stop";
static uint8_t text_DE_WarnDecoMissed[] = "Deco Stopp";
static uint8_t text_FR_WarnDecoMissed[] = "";
static uint8_t text_IT_WarnDecoMissed[] = "Deco Stop";
static uint8_t text_ES_WarnDecoMissed[] = "";

// Warning
static uint8_t text_EN_WarnFallback[] = "Fallback";
static uint8_t text_DE_WarnFallback[] = "";
static uint8_t text_FR_WarnFallback[] = "Mode repli"; // NEED to more specific here I guess...
static uint8_t text_IT_WarnFallback[] = "Ricalibrare";
static uint8_t text_ES_WarnFallback[] = "";

// Warning
static uint8_t text_EN_WarnPPO2Low[] = "O2 low";
static uint8_t text_DE_WarnPPO2Low[] = "O2 niedrig";
static uint8_t text_FR_WarnPPO2Low[] = "PpO2 basse";
static uint8_t text_IT_WarnPPO2Low[] = "PpO2 basso";
static uint8_t text_ES_WarnPPO2Low[] = "";

// Warning
static uint8_t text_EN_WarnPPO2High[] = "O2 high";
static uint8_t text_DE_WarnPPO2High[] = "O2 hoch";
static uint8_t text_FR_WarnPPO2High[] = "PpO2 haut";
static uint8_t text_IT_WarnPPO2High[] = "PpO2 Alto";
static uint8_t text_ES_WarnPPO2High[] = "";

// Warning
static uint8_t text_EN_WarnBatteryLow[] = "battery";
static uint8_t text_DE_WarnBatteryLow[] = "Batterie";
static uint8_t text_FR_WarnBatteryLow[] = "Batterie";
static uint8_t text_IT_WarnBatteryLow[] = "Batteria";
static uint8_t text_ES_WarnBatteryLow[] = "";

// Warning
static uint8_t text_EN_WarnSensorLinkLost[] = "sensors";
static uint8_t text_DE_WarnSensorLinkLost[] = "Sensoren";
static uint8_t text_FR_WarnSensorLinkLost[] = "Cellules";
static uint8_t text_IT_WarnSensorLinkLost[] = "Sensori";
static uint8_t text_ES_WarnSensorLinkLost[] = "";

// Warning
static uint8_t text_EN_WarnCnsHigh[] = "CNS high";
static uint8_t text_DE_WarnCnsHigh[] = "CNS hoch";
static uint8_t text_FR_WarnCnsHigh[] = "SNC haut";
static uint8_t text_IT_WarnCnsHigh[] = "CNS Alto";
static uint8_t text_ES_WarnCnsHigh[] = "";

// Tissue Graph
static uint8_t text_EN_Nitrogen[] = "Nitrogen";
static uint8_t text_DE_Nitrogen[] = "Stickstoff";
static uint8_t text_FR_Nitrogen[] = "Azote";
static uint8_t text_IT_Nitrogen[] = "Azoto";
static uint8_t text_ES_Nitrogen[] = "";

// Tissue Graph
static uint8_t text_EN_Helium[] = "Helium";
static uint8_t text_DE_Helium[] = "";
static uint8_t text_FR_Helium[] = "Hélium";
static uint8_t text_IT_Helium[] = "Helio";
static uint8_t text_ES_Helium[] = "";

// Tissue Graph
static uint8_t text_EN_CNS[] = "Oxygen";
static uint8_t text_DE_CNS[] = "Sauerstoff";
static uint8_t text_FR_CNS[] = "Oxygène";
static uint8_t text_IT_CNS[] = "Ossigeno";
static uint8_t text_ES_CNS[] = "";

// Profile Graph
static uint8_t text_EN_Profile[] = "Profile";
static uint8_t text_DE_Profile[] = "Profil";
static uint8_t text_FR_Profile[] = "Profil";
static uint8_t text_IT_Profile[] = "Profilo";
static uint8_t text_ES_Profile[] = "";

// Dive mode (header @ deco stop place)
static uint8_t text_EN_SafetyStop2[] = "safety stop";
static uint8_t text_DE_SafetyStop2[] = "Sicherheit";
static uint8_t text_FR_SafetyStop2[] = "Palier sécurité";
static uint8_t text_IT_SafetyStop2[] = "Tappa di sicurezza";
static uint8_t text_ES_SafetyStop2[] = "";

// Surface mode
static uint8_t text_EN_noFly[] = "noFly";
static uint8_t text_DE_noFly[] = "";
static uint8_t text_FR_noFly[] = "";  // Or Avion
static uint8_t text_IT_noFly[] = "No Volo";
static uint8_t text_ES_noFly[] = "";

// Surface mode
static uint8_t text_EN_Desaturation[] = "Desaturation";
static uint8_t text_DE_Desaturation[] = "Entsättigung";
static uint8_t text_FR_Desaturation[] = "Désaturation";
static uint8_t text_IT_Desaturation[] = "Desaturazione";
static uint8_t text_ES_Desaturation[] = "";

// Surface mode
static uint8_t text_EN_TimeSinceLastDive[] = "Last dive";
static uint8_t text_DE_TimeSinceLastDive[] = "Tauchpause";
static uint8_t text_FR_TimeSinceLastDive[] = "Intervalle";
static uint8_t text_IT_TimeSinceLastDive[] = "Ultima immersione";
static uint8_t text_ES_TimeSinceLastDive[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonLogbook[] = "Logbook";
static uint8_t text_DE_ButtonLogbook[] = "Logbuch";
static uint8_t text_FR_ButtonLogbook[] = "Carnet";
static uint8_t text_IT_ButtonLogbook[] = "Loogbook";
static uint8_t text_ES_ButtonLogbook[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonView[] = "View";
static uint8_t text_DE_ButtonView[] = "Ansicht";
static uint8_t text_FR_ButtonView[] = "Vue";
static uint8_t text_IT_ButtonView[] = "Visuale";
static uint8_t text_ES_ButtonView[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonMenu[] = "Menu";
static uint8_t text_DE_ButtonMenu[] = "Menü";
static uint8_t text_FR_ButtonMenu[] = "";// Menu
static uint8_t text_IT_ButtonMenu[] = "Menu";
static uint8_t text_ES_ButtonMenu[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonBack[] = "Back";
static uint8_t text_DE_ButtonBack[] = "Zurück";
static uint8_t text_FR_ButtonBack[] = "Retour";
static uint8_t text_IT_ButtonBack[] = "Indietro";
static uint8_t text_ES_ButtonBack[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonEnter[] = "Enter";
static uint8_t text_DE_ButtonEnter[] = "Bestätigen";
static uint8_t text_FR_ButtonEnter[] = "Entrer";
static uint8_t text_IT_ButtonEnter[] = "Conferma";
static uint8_t text_ES_ButtonEnter[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonNext[] = "Next";
static uint8_t text_DE_ButtonNext[] = "Weiter";
static uint8_t text_FR_ButtonNext[] = "Suivant"; //"Suiv.";
static uint8_t text_IT_ButtonNext[] = "Prossimo";
static uint8_t text_ES_ButtonNext[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonMinus[] = "\375";
static uint8_t text_DE_ButtonMinus[] = "";
static uint8_t text_FR_ButtonMinus[] = "";
static uint8_t text_IT_ButtonMinus[] = "";
static uint8_t text_ES_ButtonMinus[] = "";

// Button label (tiny line)
static uint8_t text_EN_ButtonPlus[] = "\373";
static uint8_t text_DE_ButtonPlus[] = "";
static uint8_t text_FR_ButtonPlus[] = "";
static uint8_t text_IT_ButtonPlus[] = "";
static uint8_t text_ES_ButtonPlus[] = "";

// Dive menu
static uint8_t text_EN_SimFollowDecoStops[] = "Ascent follows decostops";
static uint8_t text_DE_SimFollowDecoStops[] = "Sim-Aufstieg folgt Stopps";
static uint8_t text_FR_SimFollowDecoStops[] = "Remonté selon paliers";
static uint8_t text_IT_SimFollowDecoStops[] = "";
static uint8_t text_ES_SimFollowDecoStops[] = "";

// Menu SYS1
static uint8_t text_EN_Bluetooth[] = "Bluetooth";
static uint8_t text_DE_Bluetooth[] = "";
static uint8_t text_FR_Bluetooth[] = "";
static uint8_t text_IT_Bluetooth[] = "";
static uint8_t text_ES_Bluetooth[] = "";

// Customview Header
static uint8_t text_EN_Tissues[] = "tissue load";
static uint8_t text_DE_Tissues[] = "Sättigung";
static uint8_t text_FR_Tissues[] = "Saturation";
static uint8_t text_IT_Tissues[] = "Saturazione tessuti";
static uint8_t text_ES_Tissues[] = "";

// Customview Header
static uint8_t text_EN_O2monitor[] = "O2 monitor";
static uint8_t text_DE_O2monitor[] = "O2 Monitor";
static uint8_t text_FR_O2monitor[] = "Moniteur O2";  // "O2 (bar)" ?
static uint8_t text_IT_O2monitor[] = "Monitor O2";
static uint8_t text_ES_O2monitor[] = "";

// Customview Header
static uint8_t text_EN_O2voltage[] = "O2 voltage";
static uint8_t text_DE_O2voltage[] = "O2 voltage";
static uint8_t text_FR_O2voltage[] = "Voltage O2";
static uint8_t text_IT_O2voltage[] = "Voltaggio O2";
static uint8_t text_ES_O2voltage[] = "";

// Customview Header
static uint8_t text_EN_Gaslist[] = "Gaslist OC";
static uint8_t text_DE_Gaslist[] = "";
static uint8_t text_FR_Gaslist[] = "Liste gaz OC";
static uint8_t text_IT_Gaslist[] = "Lista GAS OC";
static uint8_t text_ES_Gaslist[] = "";

// Customview Header
static uint8_t text_EN_Info[] = "Info";
static uint8_t text_DE_Info[] = "";
static uint8_t text_FR_Info[] = ""; // Info
static uint8_t text_IT_Info[] = "Info";
static uint8_t text_ES_Info[] = "";

// Customview Header
static uint8_t text_EN_Warning[] = "Warning";
static uint8_t text_DE_Warning[] = "Warnung";
static uint8_t text_FR_Warning[] = "Alerte";
static uint8_t text_IT_Warning[] = "PERICOLO!";
static uint8_t text_ES_Warning[] = "";

// Customview Header (Bonex Computer)
static uint8_t text_EN_ScooterMonitor[] = "bonex";
static uint8_t text_DE_ScooterMonitor[] = "";
static uint8_t text_FR_ScooterMonitor[] = "";
static uint8_t text_IT_ScooterMonitor[] = "Bonex DPV";
static uint8_t text_ES_ScooterMonitor[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_Usage_Battery[] = "Battery life";
static uint8_t text_DE_Usage_Battery[] = "Batterie Nutzung";
static uint8_t text_FR_Usage_Battery[] = "Durée batterie";
static uint8_t text_IT_Usage_Battery[] = "Durata Batteria";
static uint8_t text_ES_Usage_Battery[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_Usage_Dives[] = "Dive Records";
static uint8_t text_DE_Usage_Dives[] = "Tauch Historie";
static uint8_t text_FR_Usage_Dives[] = "Historique plongées";
static uint8_t text_IT_Usage_Dives[] = "Totale immersioni";
static uint8_t text_ES_Usage_Dives[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_Usage_Environment[] = "Environmental conditions";
static uint8_t text_DE_Usage_Environment[] = "Umgebungsbedingungen";
static uint8_t text_FR_Usage_Environment[] = "Conditions environ.";
static uint8_t text_IT_Usage_Environment[] = "Condizioni ambientali";
static uint8_t text_ES_Usage_Environment[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_ChargeCycles[] = "charge cycles (complete)";
static uint8_t text_DE_ChargeCycles[] = "Ladezyklen (vollständig)";
static uint8_t text_FR_ChargeCycles[] = "Cycles de charge (complets)";
static uint8_t text_IT_ChargeCycles[] = "Ricarica completata";
static uint8_t text_ES_ChargeCycles[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_LowestVoltage[] = "lowest battery voltage";
static uint8_t text_DE_LowestVoltage[] = "niedriegste Batteriespannung";
static uint8_t text_FR_LowestVoltage[] = "Voltage min batterie";
static uint8_t text_IT_LowestVoltage[] = "Batteria scarica";
static uint8_t text_ES_LowestVoltage[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_HoursOfOperation[] = "hours of operation";
static uint8_t text_DE_HoursOfOperation[] = "Betriebsstunden";
static uint8_t text_FR_HoursOfOperation[] = "Heures d'utilisations";
static uint8_t text_IT_HoursOfOperation[] = "Ore di utilizzo";
static uint8_t text_ES_HoursOfOperation[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_NumberOfDives[] = "total number of dives (max. depth)";
static uint8_t text_DE_NumberOfDives[] = "Anzahl Tauchgänge (max. Tiefe)";
static uint8_t text_FR_NumberOfDives[] = "Nombre total de plongées (prof. max)";
static uint8_t text_IT_NumberOfDives[] = "Numero totale di immersioni";
static uint8_t text_ES_NumberOfDives[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_AmbientTemperature[] = "ambient temperature range";
static uint8_t text_DE_AmbientTemperature[] = "Umgebungstemperaturbereich";
static uint8_t text_FR_AmbientTemperature[] = "Temperature ambiante";   // Range --> Temperatures ?
static uint8_t text_IT_AmbientTemperature[] = "Temperatura ambiente";
static uint8_t text_ES_AmbientTemperature[] = "";

// Menu SYS2 sub Information
static uint8_t text_EN_Korrekturwerte[] = "Correction values";
static uint8_t text_DE_Korrekturwerte[] = "Korrekturwerte";
static uint8_t text_FR_Korrekturwerte[] = "Corrections";
static uint8_t text_IT_Korrekturwerte[] = "Correzione valore";
static uint8_t text_ES_Korrekturwerte[] = "";

// Customview Content
static uint8_t text_EN_Clock[] = "Clock";
static uint8_t text_DE_Clock[] = "Uhr";
static uint8_t text_FR_Clock[] = "Heure";
static uint8_t text_IT_Clock[] = "Orologio";
static uint8_t text_ES_Clock[] = "";

// Surface mode (all weekdays)
static uint8_t text_EN_Sunday[] = "Sunday";
static uint8_t text_DE_Sunday[] = "Sonntag";
static uint8_t text_FR_Sunday[] = "Dimanche";
static uint8_t text_IT_Sunday[] = "Domenica";
static uint8_t text_ES_Sunday[] = "";

static uint8_t text_EN_Monday[] = "Monday";
static uint8_t text_DE_Monday[] = "Montag";
static uint8_t text_FR_Monday[] = "Lundi";
static uint8_t text_IT_Monday[] = "Luned�";
static uint8_t text_ES_Monday[] = "";

static uint8_t text_EN_Tuesday[] = "Tuesday";
static uint8_t text_DE_Tuesday[] = "Dienstag";
static uint8_t text_FR_Tuesday[] = "Mardi";
static uint8_t text_IT_Tuesday[] = "Marted�";
static uint8_t text_ES_Tuesday[] = "";

static uint8_t text_EN_Wednesday[] = "Wednesday";
static uint8_t text_DE_Wednesday[] = "Mittwoch";
static uint8_t text_FR_Wednesday[] = "Mercredi";
static uint8_t text_IT_Wednesday[] = "Mercoled�";
static uint8_t text_ES_Wednesday[] = "";

static uint8_t text_EN_Thursday[] = "Thursday";
static uint8_t text_DE_Thursday[] = "Donnerstag";
static uint8_t text_FR_Thursday[] = "Jeudi";
static uint8_t text_IT_Thursday[] = "Gioved�";
static uint8_t text_ES_Thursday[] = "";

static uint8_t text_EN_Friday[] = "Friday";
static uint8_t text_DE_Friday[] = "Freitag";
static uint8_t text_FR_Friday[] = "Vendredi";
static uint8_t text_IT_Friday[] = "Venerd�";
static uint8_t text_ES_Friday[] = "";

static uint8_t text_EN_Saturday[] = "Saturday";
static uint8_t text_DE_Saturday[] = "Samstag";
static uint8_t text_FR_Saturday[] = "Samedi";
static uint8_t text_IT_Saturday[] = "Sabato";
static uint8_t text_ES_Saturday[] = "";

// Dive Mode und Customview Content (Bonex Computer)
static uint8_t text_EN_ScooterRestkapazitaet[] = "Battery Capacity";
static uint8_t text_DE_ScooterRestkapazitaet[] = "Batterie Kapazität";
static uint8_t text_FR_ScooterRestkapazitaet[] = "Charge Batterie";     // Ok for "...  = 35%"
static uint8_t text_IT_ScooterRestkapazitaet[] = "Carica Batteria";
static uint8_t text_ES_ScooterRestkapazitaet[] = "";

// Dive Mode und Customview Content (Bonex Computer)
static uint8_t text_EN_ScooterTemperature[] = "Motor Temp.";
static uint8_t text_DE_ScooterTemperature[] = "";
static uint8_t text_FR_ScooterTemperature[] = "Temp. moteur"; // "Température moteur";
static uint8_t text_IT_ScooterTemperature[] = "Temp. Motore";
static uint8_t text_ES_ScooterTemperature[] = "";

// Dive Mode und Customview Content (Bonex Computer)
static uint8_t text_EN_ScooterSpeed[] = "Speed";
static uint8_t text_DE_ScooterSpeed[] = "Geschwindigkeit";
static uint8_t text_FR_ScooterSpeed[] = "Vitesse";
static uint8_t text_IT_ScooterSpeed[] = "Velocit�";
static uint8_t text_ES_ScooterSpeed[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterSetup[] = "Bonex Setup";
static uint8_t text_DE_ScooterSetup[] = "";
static uint8_t text_FR_ScooterSetup[] = "Config. Bonex"; // "Configuration Bonex";
static uint8_t text_IT_ScooterSetup[] = "Settaggio DPV";
static uint8_t text_ES_ScooterSetup[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterDrag[] = "Drag";
static uint8_t text_DE_ScooterDrag[] = "";
static uint8_t text_FR_ScooterDrag[] = "";// "Traction";  // ?? No!! The traction force ? inhibition hw
static uint8_t text_IT_ScooterDrag[] = "Spinta";
static uint8_t text_ES_ScooterDrag[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterLoad[] = "Load";
static uint8_t text_DE_ScooterLoad[] = "";
static uint8_t text_FR_ScooterLoad[] = ""; // "Charge";    // ?? Kg
static uint8_t text_IT_ScooterLoad[] = "Caricamento";
static uint8_t text_ES_ScooterLoad[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterVolt[] = "Voltage";
static uint8_t text_DE_ScooterVolt[] = "Spannung";
static uint8_t text_FR_ScooterVolt[] = "";
static uint8_t text_IT_ScooterVolt[] = "Voltaggio";
static uint8_t text_ES_ScooterVolt[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterD0Apnoe[] = "Apnea";
static uint8_t text_DE_ScooterD0Apnoe[] = "Apnoe";
static uint8_t text_FR_ScooterD0Apnoe[] = "Apnée";
static uint8_t text_IT_ScooterD0Apnoe[] = "Apnea";
static uint8_t text_ES_ScooterD0Apnoe[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterD1Scuba[] = "Scuba";
static uint8_t text_DE_ScooterD1Scuba[] = "";
static uint8_t text_FR_ScooterD1Scuba[] = "Plongée";
static uint8_t text_IT_ScooterD1Scuba[] = "Immersione";
static uint8_t text_ES_ScooterD1Scuba[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterD2Tech[] = "Tech";
static uint8_t text_DE_ScooterD2Tech[] = "";
static uint8_t text_FR_ScooterD2Tech[] = ""; // Technique
static uint8_t text_IT_ScooterD2Tech[] = "Immersione tecnica";
static uint8_t text_ES_ScooterD2Tech[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterD3Heavy[] = "Heavy Tech";
static uint8_t text_DE_ScooterD3Heavy[] = "";
static uint8_t text_FR_ScooterD3Heavy[] = "";// Technique engagée
static uint8_t text_IT_ScooterD3Heavy[] = "Immersione profonda";
static uint8_t text_ES_ScooterD3Heavy[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterL0None[] = "none gear";
static uint8_t text_DE_ScooterL0None[] = "";
static uint8_t text_FR_ScooterL0None[] = "sans";
static uint8_t text_IT_ScooterL0None[] = "in Folle";
static uint8_t text_ES_ScooterL0None[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterL1Small[] = "Monotank / small double";
static uint8_t text_DE_ScooterL1Small[] = "";
static uint8_t text_FR_ScooterL1Small[] = "Mono / petit bi";
static uint8_t text_IT_ScooterL1Small[] = "Mono / Bibo";
static uint8_t text_ES_ScooterL1Small[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterL2Stages[] = "Rebreather & Stages";
static uint8_t text_DE_ScooterL2Stages[] = "";
static uint8_t text_FR_ScooterL2Stages[] = "Recycleur";
static uint8_t text_IT_ScooterL2Stages[] = "Rebreather & Stage";
static uint8_t text_ES_ScooterL2Stages[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterL3Full[] = "Multistage & Backup";
static uint8_t text_DE_ScooterL3Full[] = "";
static uint8_t text_FR_ScooterL3Full[] = "Multi-blocs & backup";
static uint8_t text_IT_ScooterL3Full[] = "Immersione multigas";
static uint8_t text_ES_ScooterL3Full[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterL4Towing[] = "Towing";
static uint8_t text_DE_ScooterL4Towing[] = "";
static uint8_t text_FR_ScooterL4Towing[] = "Remorquage";
static uint8_t text_IT_ScooterL4Towing[] = "Traino";
static uint8_t text_ES_ScooterL4Towing[] = "";

// Menu SYS1 (Bonex Computer)
static uint8_t text_EN_ScooterBattTyp[] = "Accu";
static uint8_t text_DE_ScooterBattTyp[] = "";
static uint8_t text_FR_ScooterBattTyp[] = "";
static uint8_t text_IT_ScooterBattTyp[] = "";
static uint8_t text_ES_ScooterBattTyp[] = "";

// Menu SYS1 sub (Sensors)
static uint8_t text_EN_HUDBattery[] = "HUD Battery";
static uint8_t text_DE_HUDBattery[] = "HUD Batterie";
static uint8_t text_FR_HUDBattery[] = "Batterie HUD";
static uint8_t text_IT_HUDBattery[] = "Batteria HUD";
static uint8_t text_ES_HUDBattery[] = "";

// Menu SYS1 sub (buttons)
static uint8_t text_EN_LowerIsLess[] = " lower is less sensitive";
static uint8_t text_DE_LowerIsLess[] = " niedriger ist schwergängiger";
static uint8_t text_FR_LowerIsLess[] = " diminuer moins sensible";
static uint8_t text_IT_LowerIsLess[] = "";
static uint8_t text_ES_LowerIsLess[] = "";

// Dive Mode YELLOW TEXT under Customview
static uint8_t text_EN_DiveMenuQ[] = " Menu? ";
static uint8_t text_DE_DiveMenuQ[] = " Menü? ";
static uint8_t text_FR_DiveMenuQ[] = "";
static uint8_t text_IT_DiveMenuQ[] = "Menu?";
static uint8_t text_ES_DiveMenuQ[] = "";

// Dive Mode YELLOW TEXT under Customview
static uint8_t text_EN_DiveQuitQ[] = " Quit? ";
static uint8_t text_DE_DiveQuitQ[] = " Ende? ";
static uint8_t text_FR_DiveQuitQ[] = "Quitter";
static uint8_t text_IT_DiveQuitQ[] = "Uscita?";
static uint8_t text_ES_DiveQuitQ[] = "";

// Dive Mode YELLOW TEXT under Customview
static uint8_t text_EN_DiveBearingQ[] = "Bearing";
static uint8_t text_DE_DiveBearingQ[] = "Peilung";
static uint8_t text_FR_DiveBearingQ[] = "";
static uint8_t text_IT_DiveBearingQ[] = "";
static uint8_t text_ES_DiveBearingQ[] = "";

// Dive Mode YELLOW TEXT under Customview
static uint8_t text_EN_DiveResetAvgQ[] = "ResetAvr.";
static uint8_t text_DE_DiveResetAvgQ[] = "Stoppuhr";
static uint8_t text_FR_DiveResetAvgQ[] = "";
static uint8_t text_IT_DiveResetAvgQ[] = "Reset AVR";
static uint8_t text_ES_DiveResetAvgQ[] = "";

// Menu SYS2
static uint8_t text_EN_ExtraDisplay[] = "Extra";
static uint8_t text_DE_ExtraDisplay[] = "";
static uint8_t text_FR_ExtraDisplay[] = "";
static uint8_t text_IT_ExtraDisplay[] = "Extra";
static uint8_t text_ES_ExtraDisplay[] = "";

// Menu SYS2
static uint8_t text_EN_ExtraBigFont[] = "Big Font";
static uint8_t text_DE_ExtraBigFont[] = "Grosse Schrift";
static uint8_t text_FR_ExtraBigFont[] = "Grand police";
static uint8_t text_IT_ExtraBigFont[] = "Caratteri grandi";
static uint8_t text_ES_ExtraBigFont[] = "";

// Menu SYS2 (future feature)
static uint8_t text_EN_ExtraDecoGame[] = "Deco Game";
static uint8_t text_DE_ExtraDecoGame[] = "Deko Spiel";
static uint8_t text_FR_ExtraDecoGame[] = "Jeu déco";
static uint8_t text_IT_ExtraDecoGame[] = "Giochi deco";
static uint8_t text_ES_ExtraDecoGame[] = "";

// Menu SYS2
static uint8_t text_EN_ExtraNone[] = "none";
static uint8_t text_DE_ExtraNone[] = "keines";
static uint8_t text_FR_ExtraNone[] = "sans";
static uint8_t text_IT_ExtraNone[] = "Vuoto";
static uint8_t text_ES_ExtraNone[] = "";

// Menu SYS2 Reset RTE and Firmware Update During Bluetooth Connection
static uint8_t text_EN_DecoDataLost[] = " decompression data will be lost";
static uint8_t text_DE_DecoDataLost[] = " Dekompressionsdaten verloren!";
static uint8_t text_FR_DecoDataLost[] = " RaZ de la décompression"; // RàZ
static uint8_t text_IT_DecoDataLost[] = "";
static uint8_t text_ES_DecoDataLost[] = "";

// Menu SYS1 sub and Dive Menu
static uint8_t text_EN_SetBearing[] = "Set bearing";
static uint8_t text_DE_SetBearing[] = "Kurs setzen";
static uint8_t text_FR_SetBearing[] = "Réglage cap";
static uint8_t text_IT_SetBearing[] = "";
static uint8_t text_ES_SetBearing[] = "";

static uint8_t text_EN_ResetBearing[] = "Clear bearing";
static uint8_t text_DE_ResetBearing[] = "Kurs löschen";
static uint8_t text_FR_ResetBearing[] = "Annulation cap";
static uint8_t text_IT_ResetBearing[] = "";
static uint8_t text_ES_ResetBearing[] = "";

// Menu SYS1, sub
static uint8_t text_EN_SensorList[] = "Sensor";
static uint8_t text_DE_SensorList[] = "";
static uint8_t text_FR_SensorList[] = "Cellule";
static uint8_t text_IT_SensorList[] = "";
static uint8_t text_ES_SensorList[] = "";

// Menu SYS1, sub
static uint8_t text_EN_ButtonLeft[] = "Left value";
static uint8_t text_DE_ButtonLeft[] = "Links Wert";
static uint8_t text_FR_ButtonLeft[] = "Gauge";
static uint8_t text_IT_ButtonLeft[] = "";
static uint8_t text_ES_ButtonLeft[] = "";

static uint8_t text_EN_ButtonMitte[] = "Center value";
static uint8_t text_DE_ButtonMitte[] = "Mitte Wert";
static uint8_t text_FR_ButtonMitte[] = "Centre";
static uint8_t text_IT_ButtonMitte[] = "";
static uint8_t text_ES_ButtonMitte[] = "";

static uint8_t text_EN_ButtonRight[] = "Right value";
static uint8_t text_DE_ButtonRight[] = "Rechts Wert";
static uint8_t text_FR_ButtonRight[] = "Droite";
static uint8_t text_IT_ButtonRight[] = "";
static uint8_t text_ES_ButtonRight[] = "";

// Customview in Divemode
static uint8_t text_EN_Summary[] = "Overview";
static uint8_t text_DE_Summary[] = "Uebersicht"; // T42 hat keine großen Umlaute hw 170103
static uint8_t text_FR_Summary[] = "Exposé";
static uint8_t text_IT_Summary[] = "Tutto schermo";
static uint8_t text_ES_Summary[] = "";

static uint8_t text_EN_ApneaTotal[] = "total";
static uint8_t text_DE_ApneaTotal[] = "gesamt";
static uint8_t text_FR_ApneaTotal[] = "";
static uint8_t text_IT_ApneaTotal[] = "Completo";
static uint8_t text_ES_ApneaTotal[] = "";

static uint8_t text_EN_ApneaLast[] = "last";
static uint8_t text_DE_ApneaLast[] = "letzter";
static uint8_t text_FR_ApneaLast[] = "dernier";
static uint8_t text_IT_ApneaLast[] = "";
static uint8_t text_ES_ApneaLast[] = "";

static uint8_t text_EN_ApneaSurface[] = "Surface Time";
static uint8_t text_DE_ApneaSurface[] = "Oberflächenzeit";
static uint8_t text_FR_ApneaSurface[] = "Tps.Surface";
static uint8_t text_IT_ApneaSurface[] = "Tempo di superficie";
static uint8_t text_ES_ApneaSurface[] = "";

/*
static uint8_t text_EN_ApneaCount[] = "";
static uint8_t text_DE_ApneaCount[] = "";
static uint8_t text_FR_ApneaCount[] = "";
static uint8_t text_IT_ApneaCount[] = "";
static uint8_t text_ES_ApneaCount[] = "";
*/

// -----------------------------------
// not used
// -----------------------------------


// Menu DECO1 (future)
static uint8_t text_EN_Apnoe[] = "Apnea";
static uint8_t text_DE_Apnoe[] = "Apnoe";
static uint8_t text_FR_Apnoe[] = "Apnée";
static uint8_t text_IT_Apnoe[] = "Apenea";
static uint8_t text_ES_Apnoe[] = "";

// Menu DECO1 (future)
static uint8_t text_EN_Gauge[] = "Gauge";
static uint8_t text_DE_Gauge[] = "Tiefenmesser";
static uint8_t text_FR_Gauge[] = "Profondimétre";
static uint8_t text_IT_Gauge[] = "Profondimetro";
static uint8_t text_ES_Gauge[] = "";


static uint8_t text_EN_Default[] = "Default";
static uint8_t text_DE_Default[] = "";
static uint8_t text_FR_Default[] = "Défaut";
static uint8_t text_IT_Default[] = "Standard";
static uint8_t text_ES_Default[] = "";

static uint8_t text_EN_LiterproMinute[] = "liter/minute";
static uint8_t text_DE_LiterproMinute[] = "Liter/Minute";
static uint8_t text_FR_LiterproMinute[] = "Litre/minute";
static uint8_t text_IT_LiterproMinute[] = "Litri/Miniti";
static uint8_t text_ES_LiterproMinute[] = "";

static uint8_t text_EN_Reserve[] = "Reserve";
static uint8_t text_DE_Reserve[] = "";
static uint8_t text_FR_Reserve[] = "Réserve";
static uint8_t text_IT_Reserve[] = "Riserva";
static uint8_t text_ES_Reserve[] = "";

static uint8_t text_EN_Daylightsaving[] = "Daylight sav.";
static uint8_t text_DE_Daylightsaving[] = "Sommerzeit";
static uint8_t text_FR_Daylightsaving[] = "Heure d'été";
static uint8_t text_IT_Daylightsaving[] = "";
static uint8_t text_ES_Daylightsaving[] = "";

static uint8_t text_EN_ShowDebug[] = "Debug info";
static uint8_t text_DE_ShowDebug[] = "Fehlersuche";
static uint8_t text_FR_ShowDebug[] = "Info de déboguage";
static uint8_t text_IT_ShowDebug[] = "Informazioni Debug";
static uint8_t text_ES_ShowDebug[] = "";

static uint8_t text_EN_SimTravelGas[] = "Travel Gas";
static uint8_t text_DE_SimTravelGas[] = "Reise Gas";
static uint8_t text_FR_SimTravelGas[] = "Gaz Travel";
static uint8_t text_IT_SimTravelGas[] = "Gas da viaggio";
static uint8_t text_ES_SimTravelGas[] = "";

static uint8_t text_EN_SimDecoGas[] = "Deco Gas";
static uint8_t text_DE_SimDecoGas[] = "Deko Gas";
static uint8_t text_FR_SimDecoGas[] = "Gaz déco";
static uint8_t text_IT_SimDecoGas[] = "Gas decompressivo";
static uint8_t text_ES_SimDecoGas[] = "";

static uint8_t text_EN_OTU[] = "OTU";
static uint8_t text_DE_OTU[] = "";
static uint8_t text_FR_OTU[] = "";     // UTO ?? I think we do use OTU too.
static uint8_t text_IT_OTU[] = "OTU";
static uint8_t text_ES_OTU[] = "";

/*
static uint8_t text_EN_Button1[] = "Button 1";
static uint8_t text_DE_Button1[] = "Taster 1";
static uint8_t text_FR_Button1[] = "Bouton 1";
static uint8_t text_IT_Button1[] = "Pulsante";
static uint8_t text_ES_Button1[] = "";

static uint8_t text_EN_Button2[] = "Button 2";
static uint8_t text_DE_Button2[] = "Taster 2";
static uint8_t text_FR_Button2[] = "";
static uint8_t text_IT_Button2[] = "Pulsante2";
static uint8_t text_ES_Button2[] = "";

static uint8_t text_EN_Button3[] = "Button 3";
static uint8_t text_DE_Button3[] = "Taster 3";
static uint8_t text_FR_Button3[] = "";
static uint8_t text_IT_Button3[] = "Pulsante3";
static uint8_t text_ES_Button3[] = "";

static uint8_t text_EN_Button4[] = "Button 4";
static uint8_t text_DE_Button4[] = "Taster 4";
static uint8_t text_FR_Button4[] = "";
static uint8_t text_IT_Button4[] = "Pulsante4";
static uint8_t text_ES_Button4[] = "";

static uint8_t text_EN_Yes[] = "Yes";
static uint8_t text_DE_Yes[] = "Ja";
static uint8_t text_FR_Yes[] = "Oui";
static uint8_t text_IT_Yes[] = "SI";
static uint8_t text_ES_Yes[] = "";

static uint8_t text_EN_No[] = "No";
static uint8_t text_DE_No[] = "Nein";
static uint8_t text_FR_No[] = "Non";
static uint8_t text_IT_No[] = "NO";
static uint8_t text_ES_No[] = "";

static uint8_t text_EN_Conservatism[] = "Conservatism";
static uint8_t text_DE_Conservatism[] = "Konservatismus";
static uint8_t text_FR_Conservatism[] = "Conservatisme";
static uint8_t text_IT_Conservatism[] = "Conservativismo";
static uint8_t text_ES_Conservatism[] = "";

static uint8_t text_EN_Conservatism[] = "Setting"; // was Dive Menu
static uint8_t text_DE_Conservatism[] = "Level";
static uint8_t text_FR_Conservatism[] = "Conservatisme";    // Or "Durcis."
static uint8_t text_IT_Conservatism[] = "Livello conservativismo";
static uint8_t text_ES_Conservatism[] = "";

static uint8_t text_EN_FirmwareUpdate[] = "FirmwareUpdate";
static uint8_t text_DE_FirmwareUpdate[] = "Firmware aktualisieren";
static uint8_t text_FR_FirmwareUpdate[] = "MàJ logiciel";
static uint8_t text_IT_FirmwareUpdate[] = "Firmware";
static uint8_t text_ES_FirmwareUpdate[] = "";

static uint8_t text_EN_ppo2_setting[] = "ppO2 bar";
static uint8_t text_DE_ppo2_setting[] = "";
static uint8_t text_FR_ppo2_setting[] = "PpO2 bar";
static uint8_t text_IT_ppo2_setting[] = "PpO2 Bar";
static uint8_t text_ES_ppo2_setting[] = "";

static uint8_t text_EN_down[] = "Down";
static uint8_t text_DE_down[] = "Hinunter";
static uint8_t text_FR_down[] = "";
static uint8_t text_IT_down[] = "Sotto";
static uint8_t text_ES_down[] = "";

static uint8_t text_EN_enter[] = "Enter";
static uint8_t text_DE_enter[] = "";
static uint8_t text_FR_enter[] = "Entrer";
static uint8_t text_IT_enter[] = "Conferma";
static uint8_t text_ES_enter[] = "";

static uint8_t text_EN_Change[] = "Change";
static uint8_t text_DE_Change[] = "";
static uint8_t text_FR_Change[] = "";
static uint8_t text_IT_Change[] = "Cambio";
static uint8_t text_ES_Change[] = "";
*/


// not installed 2

static uint8_t text_EN_Bottle[] = "tank size";
static uint8_t text_DE_Bottle[] = "Flasche";
static uint8_t text_FR_Bottle[] = "";
static uint8_t text_IT_Bottle[] = "Litraggio bombola";
static uint8_t text_ES_Bottle[] = "";


static uint8_t text_EN_GasVorrat[] = "Atem-Gas-Vorrat";
static uint8_t text_DE_GasVorrat[] = "Atem-Gas-Vorrat";
static uint8_t text_FR_GasVorrat[] = "";
static uint8_t text_IT_GasVorrat[] = "";
static uint8_t text_ES_GasVorrat[] = "";

static uint8_t text_EN_WirelessSender[] = "wireless id";
static uint8_t text_DE_WirelessSender[] = "Sender ID";
static uint8_t text_FR_WirelessSender[] = "";
static uint8_t text_IT_WirelessSender[] = "Numero di serie";
static uint8_t text_ES_WirelessSender[] = "";

static uint8_t text_EN_WirelessDisconnect[] = "Disconnect";
static uint8_t text_DE_WirelessDisconnect[] = "Nicht verbunden";
static uint8_t text_FR_WirelessDisconnect[] = "";
static uint8_t text_IT_WirelessDisconnect[] = "Disconnesso";
static uint8_t text_ES_WirelessDisconnect[] = "";

/*
static uint8_t text_EN_Luftintegration[] = "air integration";
static uint8_t text_DE_Luftintegration[] = "Luftintegration";
static uint8_t text_FR_Luftintegration[] = "";
static uint8_t text_IT_Luftintegration[] = "ARIA";
static uint8_t text_ES_Luftintegration[] = "";
*/

/* Lookup Table -------------------------------------------------------------*/

const tText text_array[] =
{
    {(uint8_t)TXT_Language,         {text_EN_Language, text_DE_Language, text_FR_Language, text_IT_Language, text_ES_Language}},
    {(uint8_t)TXT_LanguageName, 	{text_EN_LanguageName, text_DE_LanguageName, text_FR_LanguageName, text_IT_LanguageName, text_ES_LanguageName}},
    {(uint8_t)TXT_Depth,            {text_EN_Depth, text_DE_Depth, text_FR_Depth, text_IT_Depth, text_ES_Depth}},
    {(uint8_t)TXT_Divetime,         {text_EN_Divetime, text_DE_Divetime, text_FR_Divetime, text_IT_Divetime, text_ES_Divetime}},
    {(uint8_t)TXT_MaxDepth,         {text_EN_MaxDepth, text_DE_MaxDepth, text_FR_MaxDepth, text_IT_MaxDepth, text_ES_MaxDepth}},
    {(uint8_t)TXT_Decostop,         {text_EN_Decostop, text_DE_Decostop, text_FR_Decostop, text_IT_Decostop, text_ES_Decostop}},
    {(uint8_t)TXT_Nullzeit,         {text_EN_Nullzeit, text_DE_Nullzeit, text_FR_Nullzeit, text_IT_Nullzeit, text_ES_Nullzeit}},
    {(uint8_t)TXT_ppO2, 			{text_EN_ppO2, text_DE_ppO2, text_FR_ppO2, text_IT_ppO2, text_ES_ppO2}},
    {(uint8_t)TXT_TTS, 				{text_EN_TTS, text_DE_TTS, text_FR_TTS, text_IT_TTS, text_ES_TTS}},
    {(uint8_t)TXT_CNS, 				{text_EN_CNSshort, text_DE_CNSshort, text_FR_CNSshort, text_IT_CNSshort, text_ES_CNSshort}},
    {(uint8_t)TXT_Temperature, 		{text_EN_Temperature, text_DE_Temperature, text_FR_Temperature, text_IT_Temperature, text_ES_Temperature}},
    {(uint8_t)TXT_FutureTTS, 		{text_EN_FutureTTS, text_DE_FutureTTS, text_FR_FutureTTS, text_IT_FutureTTS, text_ES_FutureTTS}},
    {(uint8_t)TXT_AvgDepth, 		{text_EN_AvgDepth, text_DE_AvgDepth, text_FR_AvgDepth, text_IT_AvgDepth, text_ES_AvgDepth}},
    {(uint8_t)TXT_Ceiling, 			{text_EN_Ceiling, text_DE_Ceiling, text_FR_Ceiling, text_IT_Ceiling, text_ES_Ceiling}},
    {(uint8_t)TXT_ActualGradient,   {text_EN_ActualGradient, text_DE_ActualGradient, text_FR_ActualGradient, text_IT_ActualGradient, text_ES_ActualGradient}},
    {(uint8_t)TXT_Stopwatch, 		{text_EN_Stopwatch, text_DE_Stopwatch, text_FR_Stopwatch, text_IT_Stopwatch, text_ES_Stopwatch}},
    {(uint8_t)TXT_Gas, 				{text_EN_Gas, text_DE_Gas, text_FR_Gas, text_IT_Gas, text_ES_Gas}},
    {(uint8_t)TXT_Time, 			{text_EN_Time, text_DE_Time, text_FR_Time, text_IT_Time, text_ES_Time}},
    {(uint8_t)TXT_Date, 			{text_EN_Date, text_DE_Date, text_FR_Date, text_IT_Date, text_ES_Date}},
    {(uint8_t)TXT_Format, 			{text_EN_Format, text_DE_Format, text_FR_Format, text_IT_Format, text_ES_Format}},
    {(uint8_t)TXT_Warning, 			{text_EN_Warning, text_DE_Warning, text_FR_Warning, text_IT_Warning, text_ES_Warning}},
    {(uint8_t)TXT_o2Sensors, 		{text_EN_o2Sensors, text_DE_o2Sensors, text_FR_o2Sensors, text_IT_o2Sensors, text_ES_o2Sensors}},
    {(uint8_t)TXT_Brightness, 		{text_EN_Brightness, text_DE_Brightness, text_FR_Brightness, text_IT_Brightness, text_ES_Brightness}},
    {(uint8_t)TXT_Cave, 			{text_EN_Cave, text_DE_Cave, text_FR_Cave, text_IT_Cave, text_ES_Cave}},
    {(uint8_t)TXT_Eco, 				{text_EN_Eco, text_DE_Eco, text_FR_Eco, text_IT_Eco, text_ES_Eco}},
    {(uint8_t)TXT_Normal, 			{text_EN_Normal, text_DE_Normal, text_FR_Normal, text_IT_Normal, text_ES_Normal}},
    {(uint8_t)TXT_Bright, 			{text_EN_Bright, text_DE_Bright, text_FR_Bright, text_IT_Bright, text_ES_Bright}},
    {(uint8_t)TXT_Ultrabright,		{text_EN_Ultrabright, text_DE_Ultrabright, text_FR_Ultrabright, text_IT_Ultrabright, text_ES_Ultrabright}},
    {(uint8_t)TXT_OC_Gas_Edit, 		{text_EN_OC_Gas_Edit, text_DE_OC_Gas_Edit, text_FR_OC_Gas_Edit, text_IT_OC_Gas_Edit, text_ES_OC_Gas_Edit}},
    {(uint8_t)TXT_Diluent_Gas_Edit, {text_EN_Diluent_Gas_Edit, text_DE_Diluent_Gas_Edit, text_FR_Diluent_Gas_Edit, text_IT_Diluent_Gas_Edit, text_ES_Diluent_Gas_Edit}},
    {(uint8_t)TXT_Mix, 				{text_EN_Mix, text_DE_Mix, text_FR_Mix, text_IT_Mix, text_ES_Mix}},
    {(uint8_t)TXT_First, 			{text_EN_First, text_DE_First, text_FR_First, text_IT_First, text_ES_First}},
    {(uint8_t)TXT_Deco, 			{text_EN_Deco, text_DE_Deco, text_FR_Deco, text_IT_Deco, text_ES_Deco}},
    {(uint8_t)TXT_Travel, 			{text_EN_Travel, text_DE_Travel, text_FR_Travel, text_IT_Travel, text_ES_Travel}},
    {(uint8_t)TXT_Inactive, 		{text_EN_Inactive, text_DE_Inactive, text_FR_Inactive, text_IT_Inactive, text_ES_Inactive}},
    {(uint8_t)TXT_ChangeDepth, 		{text_EN_ChangeDepth, text_DE_ChangeDepth, text_FR_ChangeDepth, text_IT_ChangeDepth, text_ES_ChangeDepth}},
    {(uint8_t)TXT_Active, 			{text_EN_Active, text_DE_Active, text_FR_Active, text_IT_Active, text_ES_Active}},
    {(uint8_t)TXT_Default, 			{text_EN_Default, text_DE_Default, text_FR_Default, text_IT_Default, text_ES_Default}},
    {(uint8_t)TXT_Type, 			{text_EN_Type, text_DE_Type, text_FR_Type, text_IT_Type, text_ES_Type}},
    {(uint8_t)TXT_Setpoint_Edit,	{text_EN_Setpoint_Edit, text_DE_Setpoint_Edit, text_FR_Setpoint_Edit, text_IT_Setpoint_Edit, text_ES_Setpoint_Edit}},
    {(uint8_t)TXT_DecoAlgorithm, 	{text_EN_Algorithm, text_DE_Algorithm, text_FR_Algorithm, text_IT_Algorithm, text_ES_Algorithm}},
    {(uint8_t)TXT_ZHL16GF, 			{text_EN_ZHL16GF, text_DE_ZHL16GF, text_FR_ZHL16GF, text_IT_ZHL16GF, text_ES_ZHL16GF}},
    {(uint8_t)TXT_aGF, 				{text_EN_aGF, text_DE_aGF, text_FR_aGF, text_IT_aGF, text_ES_aGF}},
    {(uint8_t)TXT_VPM, 				{text_EN_VPM, text_DE_VPM, text_FR_VPM, text_IT_VPM, text_ES_VPM}},
    {(uint8_t)TXT_SafetyStop,		{text_EN_SafetyStop, text_DE_SafetyStop, text_FR_SafetyStop, text_IT_SafetyStop, text_ES_SafetyStop}},
    {(uint8_t)TXT_low_high, 		{text_EN_LowHigh, text_DE_LowHigh, text_FR_LowHigh, text_IT_LowHigh, text_ES_LowHigh}},
    {(uint8_t)TXT_ppO2Name, 		{text_EN_ppO2Name, text_DE_ppO2Name, text_FR_ppO2Name, text_IT_ppO2Name, text_ES_ppO2Name}},
    {(uint8_t)TXT_Maximum, 			{text_EN_Maximum, text_DE_Maximum, text_FR_Maximum, text_IT_Maximum, text_ES_Maximum}},
    {(uint8_t)TXT_Minimum,			{text_EN_Minimum, text_DE_Minimum, text_FR_Minimum, text_IT_Minimum, text_ES_Minimum}},
    {(uint8_t)TXT_Minutes, 			{text_EN_Minutes, text_DE_Minutes, text_FR_Minutes, text_IT_Minutes, text_ES_Minutes}},
    {(uint8_t)TXT_Seconds, 			{text_EN_Seconds, text_DE_Seconds, text_FR_Seconds, text_IT_Seconds, text_ES_Seconds}},
    {(uint8_t)TXT_CCRmode,			{text_EN_CCRmode, text_DE_CCRmode, text_FR_CCRmode, text_IT_CCRmode, text_ES_CCRmode}},
    {(uint8_t)TXT_AtemGasVorrat,	{text_EN_GasVorrat, text_DE_GasVorrat, text_FR_GasVorrat, text_IT_GasVorrat, text_ES_GasVorrat}},
    {(uint8_t)TXT_LiterproMinute,	{text_EN_LiterproMinute, text_DE_LiterproMinute, text_FR_LiterproMinute, text_IT_LiterproMinute, text_ES_LiterproMinute}},
    {(uint8_t)TXT_Reserve,			{text_EN_Reserve, text_DE_Reserve, text_FR_Reserve, text_IT_Reserve, text_ES_Reserve}},
    {(uint8_t)TXT_Salinity,	 		{text_EN_Salinity, text_DE_Salinity, text_FR_Salinity, text_IT_Salinity, text_ES_Salinity}},
    {(uint8_t)TXT_DiveMode,	 		{text_EN_DiveMode, text_DE_DiveMode, text_FR_DiveMode, text_IT_DiveMode, text_ES_DiveMode}},
    {(uint8_t)TXT_OpenCircuit,	 	{text_EN_OpenCircuit, text_DE_OpenCircuit, text_FR_OpenCircuit, text_IT_OpenCircuit, text_ES_OpenCircuit}},
    {(uint8_t)TXT_ClosedCircuit,	{text_EN_ClosedCircuit, text_DE_ClosedCircuit, text_FR_ClosedCircuit, text_IT_ClosedCircuit, text_ES_ClosedCircuit}},
    {(uint8_t)TXT_Apnoe,	 		{text_EN_Apnoe, text_DE_Apnoe, text_FR_Apnoe, text_IT_Apnoe, text_ES_Apnoe}},
    {(uint8_t)TXT_Gauge,	 		{text_EN_Gauge, text_DE_Gauge, text_FR_Gauge, text_IT_Gauge, text_ES_Gauge}},
    {(uint8_t)TXT_Sensor,			{text_EN_Sensor, text_DE_Sensor, text_FR_Sensor, text_IT_Sensor, text_ES_Sensor}},
    {(uint8_t)TXT_FixedSP,			{text_EN_FixedSP, text_DE_FixedSP, text_FR_FixedSP, text_IT_FixedSP, text_ES_FixedSP}},
    {(uint8_t)TXT_Decoparameters,	{text_EN_Decoparameters, text_DE_Decoparameters, text_FR_Decoparameters, text_IT_Decoparameters, text_ES_Decoparameters}},
    {(uint8_t)TXT_LastDecostop,		{text_EN_LastDecostop, text_DE_LastDecostop, text_FR_LastDecostop, text_IT_LastDecostop, text_ES_LastDecostop}},
    {(uint8_t)TXT_Fallback, 		{text_EN_Fallback, text_DE_Fallback, text_FR_Fallback, text_IT_Fallback, text_ES_Fallback}},
    {(uint8_t)TXT_DateAndTime,		{text_EN_DateTime, text_DE_DateTime, text_FR_DateTime, text_IT_DateTime, text_ES_DateTime}},
    {(uint8_t)TXT_DateConfig,		{text_EN_DayMonthYear, text_DE_DayMonthYear, text_FR_DayMonthYear, text_IT_DayMonthYear, text_ES_DayMonthYear}},
    {(uint8_t)TXT_TimeConfig,		{text_EN_StundeMinute, text_DE_StundeMinute, text_FR_StundeMinute, text_IT_StundeMinute, text_ES_StundeMinute}},
    {(uint8_t)TXT_Daylightsaving,	{text_EN_Daylightsaving, text_DE_Daylightsaving, text_FR_Daylightsaving, text_IT_Daylightsaving, text_ES_Daylightsaving}},
    {(uint8_t)TXT_Logbook,			{text_EN_Logbook, text_DE_Logbook, text_FR_Logbook, text_IT_Logbook, text_ES_Logbook}},
    {(uint8_t)TXT_LogbookEmpty,		{text_EN_LogbookEmpty, text_DE_LogbookEmpty, text_FR_LogbookEmpty, text_IT_LogbookEmpty, text_ES_LogbookEmpty}},
    {(uint8_t)TXT_Start_Calculation,{text_EN_Start_Calculation, text_DE_Start_Calculation, text_FR_Start_Calculation, text_IT_Start_Calculation, text_ES_Start_Calculation}},
    {(uint8_t)TXT_Information,		{text_EN_Information, text_DE_Information, text_FR_Information, text_IT_Information, text_ES_Information}},
    // unused{(uint8_t)TXT_Conservatism,    {text_EN_Conservatism, text_DE_Conservatism, text_FR_Conservatism, text_IT_Conservatism, text_ES_Conservatism}},
    // unused{(uint8_t)TXT_Change, 			{text_EN_Change, text_DE_Change, text_FR_Change, text_IT_Change, text_ES_Change}},
    // unused{(uint8_t)TXT_enter,			{text_EN_enter, text_DE_enter, text_FR_enter, text_IT_enter, text_ES_enter}},
    // unused{(uint8_t)TXT_down,			{text_EN_down, text_DE_down, text_FR_down, text_IT_down, text_ES_down}},
    // unused{(uint8_t)TXT_Luftintegration, {text_EN_Luftintegration, text_DE_Luftintegration, text_FR_Luftintegration, text_IT_Luftintegration, text_ES_Luftintegration}},
    // unused{(uint8_t)TXT_FirmwareUpdate,	{text_EN_FirmwareUpdate, text_DE_FirmwareUpdate, text_FR_FirmwareUpdate, text_IT_FirmwareUpdate, text_ES_FirmwareUpdate}},
    // unused{(uint8_t)TXT_ppo2_setting, 	{text_EN_ppo2_setting, text_DE_ppo2_setting, text_FR_ppo2_setting, text_IT_ppo2_setting, text_ES_ppo2_setting}},
    // unused{(uint8_t)TXT_Yes,				{text_EN_Yes, text_DE_Yes, text_FR_Yes, text_IT_Yes, text_ES_Yes}},
    // unused{(uint8_t)TXT_No,				{text_EN_No, text_DE_No, text_FR_No, text_IT_No, text_ES_No}},
};

const tText text_array2[] =
{
    {(uint8_t)TXT2BYTE_ResetMenu,		{text_EN_ResetMenu, text_DE_ResetMenu, text_FR_ResetMenu, text_IT_ResetMenu, text_ES_ResetMenu}},
    {(uint8_t)TXT2BYTE_LogbookOffset,	{text_EN_LogbookOffset, text_DE_LogbookOffset, text_FR_LogbookOffset, text_IT_LogbookOffset, text_ES_LogbookOffset}},
    {(uint8_t)TXT2BYTE_AreYouSure,		{text_EN_AreYouSure, text_DE_AreYouSure, text_FR_AreYouSure, text_IT_AreYouSure, text_ES_AreYouSure}},
    {(uint8_t)TXT2BYTE_Abort,			{text_EN_Abort, text_DE_Abort, text_FR_Abort, text_IT_Abort, text_ES_Abort}},
    {(uint8_t)TXT2BYTE_RebootRTE,		{text_EN__RebootRTE, text_DE__RebootRTE, text_FR__RebootRTE, text_IT__RebootRTE, text_ES__RebootRTE}},
    {(uint8_t)TXT2BYTE_ResetAll,		{text_EN_ResetAll, text_DE_ResetAll, text_FR_ResetAll, text_IT_ResetAll, text_ES_ResetAll}},
    {(uint8_t)TXT2BYTE_ResetDeco,		{text_EN_ResetDeco, text_DE_ResetDeco, text_FR_ResetDeco, text_IT_ResetDeco, text_ES_ResetDeco}},
    {(uint8_t)TXT2BYTE_ResetLogbook,	{text_EN_ResetLogbook, text_DE_ResetLogbook, text_FR_ResetLogbook, text_IT_ResetLogbook, text_ES_ResetLogbook}},
    {(uint8_t)TXT2BYTE_RebootMainCPU,   {text_EN_StartBootloader, text_DE_StartBootloader, text_FR_StartBootloader, text_IT_StartBootloader, text_ES_StartBootloader}},
    {(uint8_t)TXT2BYTE_Exit,			{text_EN_Exit, text_DE_Exit, text_FR_Exit, text_IT_Exit, text_ES_Exit}},
    {(uint8_t)TXT2BYTE_ShowDebug,		{text_EN_ShowDebug, text_DE_ShowDebug, text_FR_ShowDebug, text_IT_ShowDebug, text_ES_ShowDebug}},
    {(uint8_t)TXT2BYTE_PleaseUpdate,	{text_EN_PleaseUpdate, text_DE_PleaseUpdate, text_FR_PleaseUpdate, text_IT_PleaseUpdate, text_ES_PleaseUpdate}},
    {(uint8_t)TXT2BYTE_RTE,	 			{text_EN_RTE, text_DE_RTE, text_FR_RTE, text_IT_RTE, text_ES_RTE}},
    {(uint8_t)TXT2BYTE_Fonts,			{text_EN_Fonts, text_DE_Fonts, text_FR_Fonts, text_IT_Fonts, text_ES_Fonts}},
    {(uint8_t)TXT2BYTE_ResetStopwatch,  {text_EN_ResetStopwatch, text_DE_ResetStopwatch, text_FR_ResetStopwatch, text_IT_ResetStopwatch, text_ES_ResetStopwatch}},
    {(uint8_t)TXT2BYTE_SetMarker,		{text_EN_SetMarker, text_DE_SetMarker, text_FR_SetMarker, text_IT_SetMarker, text_ES_SetMarker}},
    {(uint8_t)TXT2BYTE_CompassHeading,  {text_EN_CompassHeading, text_DE_CompassHeading, text_FR_CompassHeading, text_IT_CompassHeading, text_ES_CompassHeading}},
    {(uint8_t)TXT2BYTE_Simulator,		{text_EN_Simulator, text_DE_Simulator, text_FR_Simulator, text_IT_Simulator, text_ES_Simulator}},
    {(uint8_t)TXT2BYTE_StartSimulator,  {text_EN_StartSimulator, text_DE_StartSimulator, text_FR_StartSimulator, text_IT_StartSimulator, text_ES_StartSimulator}},
    {(uint8_t)TXT2BYTE_Intervall,		{text_EN_Intervall, text_DE_Intervall, text_FR_Intervall, text_IT_Intervall, text_ES_Intervall}},
    {(uint8_t)TXT2BYTE_SimDiveTime,		{text_EN_SimDiveTime, text_DE_SimDiveTime, text_FR_SimDiveTime, text_IT_SimDiveTime, text_ES_SimDiveTime}},
    {(uint8_t)TXT2BYTE_SimMaxDepth,		{text_EN_SimMaxDepth, text_DE_SimMaxDepth, text_FR_SimMaxDepth, text_IT_SimMaxDepth, text_ES_SimMaxDepth}},
    {(uint8_t)TXT2BYTE_SimTravelGas,	{text_EN_SimTravelGas, text_DE_SimTravelGas, text_FR_SimTravelGas, text_IT_SimTravelGas, text_ES_SimTravelGas}},
    {(uint8_t)TXT2BYTE_SimDecoGas,		{text_EN_SimDecoGas, text_DE_SimDecoGas, text_FR_SimDecoGas, text_IT_SimDecoGas, text_ES_SimDecoGas}},
    {(uint8_t)TXT2BYTE_SimConsumption,  {text_EN_SimConsumption, text_DE_SimConsumption, text_FR_SimConsumption, text_IT_SimConsumption, text_ES_SimConsumption}},
    {(uint8_t)TXT2BYTE_SimSummary,		{text_EN_SimSummary, text_DE_SimSummary, text_FR_SimSummary, text_IT_SimSummary, text_ES_SimSummary}},
    {(uint8_t)TXT2BYTE_SimDecTo,		{text_EN_SimDecTo, text_DE_SimDecTo, text_FR_SimDecTo, text_IT_SimDecTo, text_ES_SimDecTo}},
    {(uint8_t)TXT2BYTE_SimLevel,		{text_EN_SimLevel, text_DE_SimLevel, text_FR_SimLevel, text_IT_SimLevel, text_ES_SimLevel}},
    {(uint8_t)TXT2BYTE_SimAscTo,		{text_EN_SimAscTo, text_DE_SimAscTo, text_FR_SimAscTo, text_IT_SimAscTo, text_ES_SimAscTo}},
    {(uint8_t)TXT2BYTE_SimSurface,		{text_EN_SimSurface, text_DE_SimSurface, text_FR_SimSurface, text_IT_SimSurface, text_ES_SimSurface}},
    {(uint8_t)TXT2BYTE_CalculateDeco,	{text_EN_CalculateDeco, text_DE_CalculateDeco, text_FR_CalculateDeco, text_IT_CalculateDeco, text_ES_CalculateDeco}},
    {(uint8_t)TXT2BYTE_Calculating,		{text_EN_Calculating, text_DE_Calculating, text_FR_Calculating, text_IT_Calculating, text_ES_Calculating}},
    {(uint8_t)TXT2BYTE_PleaseWait,		{text_EN_PleaseWait, text_DE_PleaseWait, text_FR_PleaseWait, text_IT_PleaseWait, text_ES_PleaseWait}},
    {(uint8_t)TXT2BYTE_Decolist,		{text_EN_Decolist, text_DE_Decolist, text_FR_Decolist, text_IT_Decolist, text_ES_Decolist}},
    {(uint8_t)TXT2BYTE_ButtonSensitivity,{text_EN_ButtonSensitivity, text_DE_ButtonSensitivity, text_FR_ButtonSensitivity, text_IT_ButtonSensitivity, text_ES_ButtonSensitivity}},
    {(uint8_t)TXT2BYTE_SpecialDiveGas,	{text_EN_SpecialDiveGas, text_DE_SpecialDiveGas, text_FR_SpecialDiveGas, text_IT_SpecialDiveGas, text_ES_SpecialDiveGas}},
    {(uint8_t)TXT2BYTE_SpecialDiveGasMenu,{text_EN_SpecialDiveGasMenu, text_DE_SpecialDiveGasMenu, text_FR_SpecialDiveGasMenu, text_IT_SpecialDiveGasMenu, text_ES_SpecialDiveGasMenu}},
    {(uint8_t)TXT2BYTE_SpecialDiveGasMenuCCR,{text_EN_SpecialDiveGasMenuCCR, text_DE_SpecialDiveGasMenuCCR, text_FR_SpecialDiveGasMenuCCR, text_IT_SpecialDiveGasMenuCCR, text_ES_SpecialDiveGasMenuCCR}},
    {(uint8_t)TXT2BYTE_CompassCalib, 	{text_EN_CompassCalib, text_DE_CompassCalib, text_FR_CompassCalib, text_IT_CompassCalib, text_ES_CompassCalib}},
    {(uint8_t)TXT2BYTE_UseSensor,		{text_EN_UseSensor, text_DE_UseSensor, text_FR_UseSensor, text_IT_UseSensor, text_ES_UseSensor}},
    {(uint8_t)TXT2BYTE_WarnDecoMissed,  {text_EN_WarnDecoMissed, text_DE_WarnDecoMissed, text_FR_WarnDecoMissed, text_IT_WarnDecoMissed, text_ES_WarnDecoMissed}},
    {(uint8_t)TXT2BYTE_WarnPPO2Low,		{text_EN_WarnPPO2Low, text_DE_WarnPPO2Low, text_FR_WarnPPO2Low, text_IT_WarnPPO2Low, text_ES_WarnPPO2Low}},
    {(uint8_t)TXT2BYTE_WarnPPO2High,	{text_EN_WarnPPO2High, text_DE_WarnPPO2High, text_FR_WarnPPO2High, text_IT_WarnPPO2High, text_ES_WarnPPO2High}},
    {(uint8_t)TXT2BYTE_WarnBatteryLow,  {text_EN_WarnBatteryLow, text_DE_WarnBatteryLow, text_FR_WarnBatteryLow, text_IT_WarnBatteryLow, text_ES_WarnBatteryLow}},
    {(uint8_t)TXT2BYTE_WarnSensorLinkLost,{text_EN_WarnSensorLinkLost, text_DE_WarnSensorLinkLost, text_FR_WarnSensorLinkLost, text_IT_WarnSensorLinkLost, text_ES_WarnSensorLinkLost}},
    {(uint8_t)TXT2BYTE_WarnFallback,	{text_EN_WarnFallback, text_DE_WarnFallback, text_FR_WarnFallback, text_IT_WarnFallback, text_ES_WarnFallback}},
    {(uint8_t)TXT2BYTE_WarnCnsHigh,		{text_EN_WarnCnsHigh, text_DE_WarnCnsHigh, text_FR_WarnCnsHigh, text_IT_WarnCnsHigh, text_ES_WarnCnsHigh}},
    {(uint8_t)TXT2BYTE_O2monitor,		{text_EN_O2monitor, text_DE_O2monitor, text_FR_O2monitor, text_IT_O2monitor, text_ES_O2monitor}},
    {(uint8_t)TXT2BYTE_O2voltage,		{text_EN_O2voltage, text_DE_O2voltage, text_FR_O2voltage, text_IT_O2voltage, text_ES_O2voltage}},
    {(uint8_t)TXT2BYTE_Tissues,			{text_EN_Tissues, text_DE_Tissues, text_FR_Tissues, text_IT_Tissues, text_ES_Tissues}},
    {(uint8_t)TXT2BYTE_Nitrogen,		{text_EN_Nitrogen, text_DE_Nitrogen, text_FR_Nitrogen, text_IT_Nitrogen, text_ES_Nitrogen}},
    {(uint8_t)TXT2BYTE_Helium,			{text_EN_Helium, text_DE_Helium, text_FR_Helium, text_IT_Helium, text_ES_Helium}},
    {(uint8_t)TXT2BYTE_CNS,				{text_EN_CNS, text_DE_CNS, text_FR_CNS, text_IT_CNS, text_ES_CNS}},
    {(uint8_t)TXT2BYTE_OTU,				{text_EN_OTU, text_DE_OTU, text_FR_OTU, text_IT_OTU, text_ES_OTU}},
    {(uint8_t)TXT2BYTE_Profile,			{text_EN_Profile, text_DE_Profile, text_FR_Profile, text_IT_Profile, text_ES_Profile}},
    {(uint8_t)TXT2BYTE_Compass, 		{text_EN_Compass, text_DE_Compass, text_FR_Compass, text_IT_Compass, text_ES_Compass}},
    {(uint8_t)TXT2BYTE_SafetyStop2,		{text_EN_SafetyStop2, text_DE_SafetyStop2, text_FR_SafetyStop2, text_IT_SafetyStop2, text_ES_SafetyStop2}},
    {(uint8_t)TXT2BYTE_noFly,			{text_EN_noFly, text_DE_noFly, text_FR_noFly, text_IT_noFly, text_ES_noFly}},
    {(uint8_t)TXT2BYTE_Desaturation,	{text_EN_Desaturation, text_DE_Desaturation, text_FR_Desaturation, text_IT_Desaturation, text_ES_Desaturation}},
    {(uint8_t)TXT2BYTE_TimeSinceLastDive,{text_EN_TimeSinceLastDive, text_DE_TimeSinceLastDive, text_FR_TimeSinceLastDive, text_IT_TimeSinceLastDive, text_ES_TimeSinceLastDive}},
    {(uint8_t)TXT2BYTE_ButtonLogbook,	{text_EN_ButtonLogbook, text_DE_ButtonLogbook, text_FR_ButtonLogbook, text_IT_ButtonLogbook, text_ES_ButtonLogbook}},
    {(uint8_t)TXT2BYTE_ButtonMenu,		{text_EN_ButtonMenu, text_DE_ButtonMenu, text_FR_ButtonMenu, text_IT_ButtonMenu, text_ES_ButtonMenu}},
    {(uint8_t)TXT2BYTE_ButtonView,		{text_EN_ButtonView, text_DE_ButtonView, text_FR_ButtonView, text_IT_ButtonView, text_ES_ButtonView}},
    {(uint8_t)TXT2BYTE_ButtonBack,		{text_EN_ButtonBack, text_DE_ButtonBack, text_FR_ButtonBack, text_IT_ButtonBack, text_ES_ButtonBack}},
    {(uint8_t)TXT2BYTE_ButtonEnter,		{text_EN_ButtonEnter, text_DE_ButtonEnter, text_FR_ButtonEnter, text_IT_ButtonEnter, text_ES_ButtonEnter}},
    {(uint8_t)TXT2BYTE_ButtonNext,		{text_EN_ButtonNext, text_DE_ButtonNext, text_FR_ButtonNext, text_IT_ButtonNext, text_ES_ButtonNext}},
    {(uint8_t)TXT2BYTE_ButtonMinus,		{text_EN_ButtonMinus, text_DE_ButtonMinus, text_FR_ButtonMinus, text_IT_ButtonMinus, text_ES_ButtonMinus}},
    {(uint8_t)TXT2BYTE_ButtonPlus,		{text_EN_ButtonPlus, text_DE_ButtonPlus, text_FR_ButtonPlus, text_IT_ButtonPlus, text_ES_ButtonPlus}},
    {(uint8_t)TXT2BYTE_SimFollowDecoStops,{text_EN_SimFollowDecoStops, text_DE_SimFollowDecoStops, text_FR_SimFollowDecoStops, text_IT_SimFollowDecoStops, text_ES_SimFollowDecoStops}},
    {(uint8_t)TXT2BYTE_Bluetooth,		{text_EN_Bluetooth, text_DE_Bluetooth, text_FR_Bluetooth, text_IT_Bluetooth, text_ES_Bluetooth}},
    {(uint8_t)TXT2BYTE_Usage_Battery,	{text_EN_Usage_Battery, text_DE_Usage_Battery, text_FR_Usage_Battery, text_IT_Usage_Battery, text_ES_Usage_Battery}},
    {(uint8_t)TXT2BYTE_Usage_Dives,		{text_EN_Usage_Dives, text_DE_Usage_Dives, text_FR_Usage_Dives, text_IT_Usage_Dives, text_ES_Usage_Dives}},
    {(uint8_t)TXT2BYTE_Usage_Environment,{text_EN_Usage_Environment, text_DE_Usage_Environment, text_FR_Usage_Environment, text_IT_Usage_Environment, text_ES_Usage_Environment}},
    {(uint8_t)TXT2BYTE_ChargeCycles,	{text_EN_ChargeCycles, text_DE_ChargeCycles, text_FR_ChargeCycles, text_IT_ChargeCycles, text_ES_ChargeCycles}},
    {(uint8_t)TXT2BYTE_LowestVoltage,	{text_EN_LowestVoltage, text_DE_LowestVoltage, text_FR_LowestVoltage, text_IT_LowestVoltage, text_ES_LowestVoltage}},
    {(uint8_t)TXT2BYTE_HoursOfOperation,{text_EN_HoursOfOperation, text_DE_HoursOfOperation, text_FR_HoursOfOperation, text_IT_HoursOfOperation, text_ES_HoursOfOperation}},
    {(uint8_t)TXT2BYTE_NumberOfDives,	{text_EN_NumberOfDives, text_DE_NumberOfDives, text_FR_NumberOfDives, text_IT_NumberOfDives, text_ES_NumberOfDives}},
    {(uint8_t)TXT2BYTE_AmbientTemperature,{text_EN_AmbientTemperature, text_DE_AmbientTemperature, text_FR_AmbientTemperature, text_IT_AmbientTemperature, text_ES_AmbientTemperature}},
    {(uint8_t)TXT2BYTE_Bottle,			{text_EN_Bottle, text_DE_Bottle, text_FR_Bottle, text_IT_Bottle, text_ES_Bottle}},
    {(uint8_t)TXT2BYTE_WirelessSender,{text_EN_WirelessSender, text_DE_WirelessSender, text_FR_WirelessSender, text_IT_WirelessSender, text_ES_WirelessSender}},
    {(uint8_t)TXT2BYTE_WirelessDisconnect,{text_EN_WirelessDisconnect, text_DE_WirelessDisconnect, text_FR_WirelessDisconnect, text_IT_WirelessDisconnect, text_ES_WirelessDisconnect}},
    {(uint8_t)TXT2BYTE_Gaslist,			{text_EN_Gaslist, text_DE_Gaslist, text_FR_Gaslist, text_IT_Gaslist, text_ES_Gaslist}},
    {(uint8_t)TXT2BYTE_Clock,			{text_EN_Clock, text_DE_Clock, text_FR_Clock, text_IT_Clock, text_ES_Clock}},
    {(uint8_t)TXT2BYTE_Sunday,			{text_EN_Sunday, text_DE_Sunday, text_FR_Sunday, text_IT_Sunday, text_ES_Sunday}},
    {(uint8_t)TXT2BYTE_Monday,			{text_EN_Monday, text_DE_Monday, text_FR_Monday, text_IT_Monday, text_ES_Monday}},
    {(uint8_t)TXT2BYTE_Tuesday,			{text_EN_Tuesday, text_DE_Tuesday, text_FR_Tuesday, text_IT_Tuesday, text_ES_Tuesday}},
    {(uint8_t)TXT2BYTE_Wednesday,		{text_EN_Wednesday, text_DE_Wednesday, text_FR_Wednesday, text_IT_Wednesday, text_ES_Wednesday}},
    {(uint8_t)TXT2BYTE_Thursday,		{text_EN_Thursday, text_DE_Thursday, text_FR_Thursday, text_IT_Thursday, text_ES_Thursday}},
    {(uint8_t)TXT2BYTE_Friday,			{text_EN_Friday, text_DE_Friday, text_FR_Friday, text_IT_Friday, text_ES_Friday}},
    {(uint8_t)TXT2BYTE_Saturday,		{text_EN_Saturday, text_DE_Saturday, text_FR_Saturday, text_IT_Saturday, text_ES_Saturday}},
    {(uint8_t)TXT2BYTE_ScooterRestkapazitaet,{text_EN_ScooterRestkapazitaet, text_DE_ScooterRestkapazitaet, text_FR_ScooterRestkapazitaet, text_IT_ScooterRestkapazitaet, text_ES_ScooterRestkapazitaet}},
    {(uint8_t)TXT2BYTE_ScooterTemperature,{text_EN_ScooterTemperature, text_DE_ScooterTemperature, text_FR_ScooterTemperature, text_IT_ScooterTemperature, text_ES_ScooterTemperature}},
    {(uint8_t)TXT2BYTE_ScooterSpeed,	{text_EN_ScooterSpeed, text_DE_ScooterSpeed, text_FR_ScooterSpeed, text_IT_ScooterSpeed, text_ES_ScooterSpeed}},
    {(uint8_t)TXT2BYTE_Layout,			{text_EN_Design, text_DE_Design, text_FR_Design, text_IT_Design, text_ES_Design}},
    {(uint8_t)TXT2BYTE_Units,			{text_EN_Units, text_DE_Units, text_FR_Units, text_IT_Units, text_ES_Units}},
    {(uint8_t)TXT2BYTE_Units_metric,	{text_EN_Design_t7_metric, text_DE_Design_t7_metric, text_FR_Design_t7_metric, text_IT_Design_t7_metric, text_ES_Design_t7_metric}},
    {(uint8_t)TXT2BYTE_Units_feet,		{text_EN_Design_t7_feet, text_DE_Design_t7_feet, text_FR_Design_t7_feet, text_IT_Design_t7_feet, text_ES_Design_t7_feet}},
    {(uint8_t)TXT2BYTE_Farbschema,		{text_EN_Farbschema, text_DE_Farbschema, text_FR_Farbschema, text_IT_Farbschema, text_ES_Farbschema}},
    {(uint8_t)TXT2BYTE_Customviews,		{text_EN_Customviews, text_DE_Customviews, text_FR_Customviews, text_IT_Customviews, text_ES_Customviews}},
    {(uint8_t)TXT2BYTE_CViewTimeout,	{text_EN_CViewTimeout, text_DE_CViewTimeout, text_FR_CViewTimeout, text_IT_CViewTimeout, text_ES_CViewTimeout}},
    {(uint8_t)TXT2BYTE_CViewStandard,	{text_EN_CViewStandard, text_DE_CViewStandard, text_FR_CViewStandard, text_IT_CViewStandard, text_ES_CViewStandard}},
    {(uint8_t)TXT2BYTE_CornerTimeout,   {text_EN_CornerTimeout, text_DE_CornerTimeout, text_FR_CornerTimeout, text_IT_CornerTimeout, text_ES_CornerTimeout}},
    {(uint8_t)TXT2BYTE_CornerStandard,  {text_EN_CornerStandard, text_DE_CornerStandard, text_FR_CornerStandard, text_IT_CornerStandard, text_ES_CornerStandard}},
    {(uint8_t)TXT2BYTE_SetToMOD,		{text_EN_SetToMOD, text_DE_SetToMOD, text_FR_SetToMOD, text_IT_SetToMOD, text_ES_SetToMOD}},
    {(uint8_t)TXT2BYTE_HUDbattery,		{text_EN_HUDBattery, text_DE_HUDBattery, text_FR_HUDBattery, text_IT_HUDBattery, text_ES_HUDBattery}},
    {(uint8_t)TXT2BYTE_LowerIsLess,		{text_EN_LowerIsLess, text_DE_LowerIsLess, text_FR_LowerIsLess, text_IT_LowerIsLess, text_ES_LowerIsLess}},
    {(uint8_t)TXT2BYTE_DiveMenuQ,		{text_EN_DiveMenuQ, text_DE_DiveMenuQ, text_FR_DiveMenuQ, text_IT_DiveMenuQ, text_ES_DiveMenuQ}},
    {(uint8_t)TXT2BYTE_DiveQuitQ,		{text_EN_DiveQuitQ, text_DE_DiveQuitQ, text_FR_DiveQuitQ, text_IT_DiveQuitQ, text_ES_DiveQuitQ}},
    {(uint8_t)TXT2BYTE_DiveBearingQ,	{text_EN_DiveBearingQ, text_DE_DiveBearingQ, text_FR_DiveBearingQ, text_IT_DiveBearingQ, text_ES_DiveBearingQ}},
    {(uint8_t)TXT2BYTE_DiveResetAvgQ,	{text_EN_DiveResetAvgQ, text_DE_DiveResetAvgQ, text_FR_DiveResetAvgQ, text_IT_DiveResetAvgQ, text_ES_DiveResetAvgQ}},
    {(uint8_t)TXT2BYTE_ExtraDisplay,	{text_EN_ExtraDisplay, text_DE_ExtraDisplay, text_FR_ExtraDisplay, text_IT_ExtraDisplay, text_ES_ExtraDisplay}},
    {(uint8_t)TXT2BYTE_ExtraBigFont,	{text_EN_ExtraBigFont, text_DE_ExtraBigFont, text_FR_ExtraBigFont, text_IT_ExtraBigFont, text_ES_ExtraBigFont}},
    {(uint8_t)TXT2BYTE_ExtraDecoGame,	{text_EN_ExtraDecoGame, text_DE_ExtraDecoGame, text_FR_ExtraDecoGame, text_IT_ExtraDecoGame, text_ES_ExtraDecoGame}},
    {(uint8_t)TXT2BYTE_ExtraNone,		{text_EN_ExtraNone, text_DE_ExtraNone, text_FR_ExtraNone, text_IT_ExtraNone, text_ES_ExtraNone}},
    {(uint8_t)TXT2BYTE_DecoDataLost,	{text_EN_DecoDataLost, text_DE_DecoDataLost, text_FR_DecoDataLost, text_IT_DecoDataLost, text_ES_DecoDataLost}},
    {(uint8_t)TXT2BYTE_Info,			{text_EN_Info, text_DE_Info, text_FR_Info, text_IT_Info, text_ES_Info}},
    {(uint8_t)TXT2BYTE_Korrekturwerte,  {text_EN_Korrekturwerte, text_DE_Korrekturwerte, text_FR_Korrekturwerte, text_IT_Korrekturwerte, text_ES_Korrekturwerte}},
    {(uint8_t)TXT2BYTE_ScooterMonitor,  {text_EN_ScooterMonitor, text_DE_ScooterMonitor, text_FR_ScooterMonitor, text_IT_ScooterMonitor, text_ES_ScooterMonitor}},
    {(uint8_t)TXT2BYTE_SetBearing,		{text_EN_SetBearing, text_DE_SetBearing, text_FR_SetBearing, text_IT_SetBearing, text_ES_SetBearing}},
    {(uint8_t)TXT2BYTE_ResetBearing,	{text_EN_ResetBearing, text_DE_ResetBearing, text_FR_ResetBearing, text_IT_ResetBearing, text_ES_ResetBearing}},
    {(uint8_t)TXT2BYTE_ScooterSetup,	{text_EN_ScooterSetup, text_DE_ScooterSetup, text_FR_ScooterSetup, text_IT_ScooterSetup, text_ES_ScooterSetup}},
    {(uint8_t)TXT2BYTE_ScooterDrag,		{text_EN_ScooterDrag, text_DE_ScooterDrag, text_FR_ScooterDrag, text_IT_ScooterDrag, text_ES_ScooterDrag}},
    {(uint8_t)TXT2BYTE_ScooterLoad,		{text_EN_ScooterLoad, text_DE_ScooterLoad, text_FR_ScooterLoad, text_IT_ScooterLoad, text_ES_ScooterLoad}},
    {(uint8_t)TXT2BYTE_ScooterD0Apnoe,  {text_EN_ScooterD0Apnoe, text_DE_ScooterD0Apnoe, text_FR_ScooterD0Apnoe, text_IT_ScooterD0Apnoe, text_ES_ScooterD0Apnoe}},
    {(uint8_t)TXT2BYTE_ScooterD1Scuba,  {text_EN_ScooterD1Scuba, text_DE_ScooterD1Scuba, text_FR_ScooterD1Scuba, text_IT_ScooterD1Scuba, text_ES_ScooterD1Scuba}},
    {(uint8_t)TXT2BYTE_ScooterD2Tech,	{text_EN_ScooterD2Tech, text_DE_ScooterD2Tech, text_FR_ScooterD2Tech, text_IT_ScooterD2Tech, text_ES_ScooterD2Tech}},
    {(uint8_t)TXT2BYTE_ScooterD3Heavy,  {text_EN_ScooterD3Heavy, text_DE_ScooterD3Heavy, text_FR_ScooterD3Heavy, text_IT_ScooterD3Heavy, text_ES_ScooterD3Heavy}},
    {(uint8_t)TXT2BYTE_ScooterL0None,	{text_EN_ScooterL0None, text_DE_ScooterL0None, text_FR_ScooterL0None, text_IT_ScooterL0None, text_ES_ScooterL0None}},
    {(uint8_t)TXT2BYTE_ScooterL1Small,  {text_EN_ScooterL1Small, text_DE_ScooterL1Small, text_FR_ScooterL1Small, text_IT_ScooterL1Small, text_ES_ScooterL1Small}},
    {(uint8_t)TXT2BYTE_ScooterL2Stages, {text_EN_ScooterL2Stages, text_DE_ScooterL2Stages, text_FR_ScooterL2Stages, text_IT_ScooterL2Stages, text_ES_ScooterL2Stages}},
    {(uint8_t)TXT2BYTE_ScooterL3Full,	{text_EN_ScooterL3Full, text_DE_ScooterL3Full, text_FR_ScooterL3Full, text_IT_ScooterL3Full, text_ES_ScooterL3Full}},
    {(uint8_t)TXT2BYTE_ScooterL4Towing, {text_EN_ScooterL4Towing, text_DE_ScooterL4Towing, text_FR_ScooterL4Towing, text_IT_ScooterL4Towing, text_ES_ScooterL4Towing}},
    {(uint8_t)TXT2BYTE_ScooterBattTyp,  {text_EN_ScooterBattTyp, text_DE_ScooterBattTyp, text_FR_ScooterBattTyp, text_IT_ScooterBattTyp, text_ES_ScooterBattTyp}},
    {(uint8_t)TXT2BYTE_ScooterVolt,		{text_EN_ScooterVolt, text_DE_ScooterVolt, text_FR_ScooterVolt, text_IT_ScooterVolt, text_ES_ScooterVolt}},
    {(uint8_t)TXT2BYTE_Sensor,			{text_EN_SensorList, text_DE_SensorList, text_FR_SensorList, text_IT_SensorList, text_ES_SensorList}},
    {(uint8_t)TXT2BYTE_Maintenance,		{text_EN_Maintenance, text_DE_Maintenance, text_FR_Maintenance, text_IT_Maintenance, text_ES_Maintenance}},
    {(uint8_t)TXT2BYTE_SetBatteryCharge,{text_EN_SetBatteryCharge, text_DE_SetBatteryCharge, text_FR_SetBatteryCharge, text_IT_SetBatteryCharge, text_ES_SetBatteryCharge}},
    {(uint8_t)TXT2BYTE_SetFactoryDefaults,{text_EN_SetFactoryDefaults, text_DE_SetFactoryDefaults, text_FR_SetFactoryDefaults, text_IT_SetFactoryDefaults, text_ES_SetFactoryDefaults}},


    {(uint8_t)TXT2BYTE_Reboot,			{text_EN_Reboot, text_DE_Reboot, text_FR_Reboot, text_IT_Reboot, text_ES_Reboot}},
    {(uint8_t)TXT2BYTE_ButtonLeft,		{text_EN_ButtonLeft, text_DE_ButtonLeft, text_FR_ButtonLeft, text_IT_ButtonLeft, text_ES_ButtonLeft}},
    {(uint8_t)TXT2BYTE_ButtonMitte,		{text_EN_ButtonMitte, text_DE_ButtonMitte, text_FR_ButtonMitte, text_IT_ButtonMitte, text_ES_ButtonMitte}},
    {(uint8_t)TXT2BYTE_ButtonRight,		{text_EN_ButtonRight, text_DE_ButtonRight, text_FR_ButtonRight, text_IT_ButtonRight, text_ES_ButtonRight}},
    {(uint8_t)TXT2BYTE_Summary,			{text_EN_Summary, text_DE_Summary, text_FR_Summary, text_IT_Summary, text_ES_Summary}},
    {(uint8_t)TXT2BYTE_ApneaLast,		{text_EN_ApneaLast, text_DE_ApneaLast, text_FR_ApneaLast, text_IT_ApneaLast, text_ES_ApneaLast}},
    {(uint8_t)TXT2BYTE_ApneaTotal,		{text_EN_ApneaTotal, text_DE_ApneaTotal, text_FR_ApneaTotal, text_IT_ApneaTotal, text_ES_ApneaTotal}},
    {(uint8_t)TXT2BYTE_ApneaSurface,	{text_EN_ApneaSurface, text_DE_ApneaSurface, text_FR_ApneaSurface, text_IT_ApneaSurface, text_ES_ApneaSurface}},

//	{(uint8_t)TXT2BYTE_ApneaCount,          {text_EN_ApneaCount, text_DE_ApneaCount, text_FR_ApneaCount, text_IT_ApneaCount, text_ES_ApneaCount}},
    // unused {(uint8_t)TXT2BYTE_Button1,	{text_EN_Button1, text_DE_Button1, text_FR_Button1, text_IT_Button1, text_ES_Button1}},
    // unused {(uint8_t)TXT2BYTE_Button2,	{text_EN_Button2, text_DE_Button2, text_FR_Button2, text_IT_Button2, text_ES_Button2}},
    // unused {(uint8_t)TXT2BYTE_Button3,	{text_EN_Button3, text_DE_Button3, text_FR_Button3, text_IT_Button3, text_ES_Button3}},
    // unused {(uint8_t)TXT2BYTE_Button4,	{text_EN_Button4, text_DE_Button4, text_FR_Button4, text_IT_Button4, text_ES_Button4}},
};

