*** D:\userfile\ptagis\programs\COMPASS Flow Mod 70 yr test\PImotion1 and 2 2008-12-16\PImotion3.sas;
*** v5 - reads CSV file for spill shaper ***;
*** adds min turb flows for every project (v4) per KAF 6-1 11:00 PRD BOGUS NUMBER FOR THIS;
*** drop day-night stuff (v3);
*** May 31 - trial of within-month spill shaping ;
*** Mar. 22 - production run for Susannah and Jim Faulkner ;
** Mar 21 - per call with S. I. and 2nd call with Kim, set day percent spill = nite;
** This avoids spilling > 100 percent of river;
*** Mar 19 - 2007 test, uses 2004 day-nite split for now;
** MAR 19 2007 ** ;
** adds Hells Canyon to mix 03-19;
*** drop creation of data for hand match-up ;
** Per KAF 3-14 e-amil, does day-nite spill splits;
** per 3-14 RD e-mail, does ANAone and Peck too;
** per 3-14 phone call to S. I., adds monthly elevation;
** Matches HYDSIM and real years based on Apr-June flows per KF 3-2 AM call and e-mail ;
** automates HYDSIM vs real flow year matching (for now) ;

options errors = 3 mprint ;

title1 "D:\userfile\ptagis\programs\COMPASS Flow Mod 70 yr test\PImotion1 and 2 2008-12-16\PImotion3.sas";
libname sasdata "D:\userfile\ptagis\programs\COMPASS Flow Mod 70 yr test\sasdata";
/***********  NO DAY-NIGHT  - just set equal 6-1   **********/
%macro proj_to_SI (proj, pname);
	%let tot_d = _tot_Q_daily ;
	%let spill_d = _spill_day_daily;
	%let spill_n = _spill_nite_daily;
	%let elev = _elev_feet;

	/*** daily_flow_kcfs 
		day_spill_proporion 
		night_spill_proportion
		monthly_elevation_feet ***/

	project = "&pname";
	if project = "ICH" then project = "IHR" ; ** per S. I. 3-21 ;
	if project = "ANA" or project = "PEC" then do ;
		&proj&spill_d = 0;
		&proj&spill_n = 0 ;
		&proj&elev = . ;
	end;
	daily_flow_kcfs = &proj&tot_d ;
	turb_q_daily = daily_flow_kcfs - &proj&spill_d ;
	day_spill_proportion = &proj&spill_d / daily_flow_kcfs ;
	night_spill_proportion = &proj&spill_n / daily_flow_kcfs ;
	monthly_elevation_feet = &proj&elev;
/*********** 6-1 kluge to max turb flow non-negative ***/
	if project = "BON" and turb_q_daily < 30 then do; ** Numbers for min turb fr KAF 6-1;
		turb_q_daily = min (30, daily_flow_kcfs  );
		&proj&spill_d = daily_flow_kcfs  - turb_q_daily ;
		&proj&spill_n = daily_flow_kcfs  - turb_q_daily ;
		day_spill_proportion = &proj&spill_d / daily_flow_kcfs ;
		night_spill_proportion = &proj&spill_n / daily_flow_kcfs ;
	end;
	if (project = "TDA" or project = "JDA" or project = "MCN") and turb_q_daily < 50 then do; ** Numbers for min turb fr KAF 6-1;
		turb_q_daily = min (50, daily_flow_kcfs  );
		&proj&spill_d = daily_flow_kcfs  - turb_q_daily ;
		&proj&spill_n = daily_flow_kcfs  - turb_q_daily ;
		day_spill_proportion = &proj&spill_d / daily_flow_kcfs ;
		night_spill_proportion = &proj&spill_n / daily_flow_kcfs ;
	end;
	if project = "IHR" and turb_q_daily < 9.5 then do; ** Numbers for min turb fr KAF 6-1;
		turb_q_daily = min (9.5, daily_flow_kcfs  );
		&proj&spill_d = daily_flow_kcfs  - turb_q_daily ;
		&proj&spill_n = daily_flow_kcfs  - turb_q_daily ;
		day_spill_proportion = &proj&spill_d / daily_flow_kcfs ;
		night_spill_proportion = &proj&spill_n / daily_flow_kcfs ;

	end;
	if (project = "LWG" or project = "LMN" or project = "LGS") and turb_q_daily < 11.5 then do; ** Numbers for min turb fr KAF 6-1;
		turb_q_daily = min (11.5, daily_flow_kcfs  );
		&proj&spill_d = daily_flow_kcfs  - turb_q_daily ;
		&proj&spill_n = daily_flow_kcfs  - turb_q_daily ;
		day_spill_proportion = &proj&spill_d / daily_flow_kcfs ;
		night_spill_proportion = &proj&spill_n / daily_flow_kcfs ;
	end;
	if project = "PRD" and turb_q_daily < 50 then do; ** BOGUS NUMBER HERE 6-1;
		turb_q_daily = min (50, daily_flow_kcfs  );
		&proj&spill_d = daily_flow_kcfs  - turb_q_daily ;
		&proj&spill_n = daily_flow_kcfs  - turb_q_daily ;
		day_spill_proportion = &proj&spill_d / daily_flow_kcfs ;
		night_spill_proportion = &proj&spill_n / daily_flow_kcfs ;
	end;

	if (turb_q_daily < 0 and julian_day > 90 and julian_day < 180)
			then put project= cal_yr= julian_day= daily_flow_kcfs= day_spill_proportion= ;
	spill_prob = 0;
	if day_spill_proportion > 0.999 or night_spill_proportion > 0.999 then spill_prob = 1;
	if spill_prob = 1 then do;
		put "SPILL PROBLEM" ;
		put project= cal_yr= julian_day= daily_flow_kcfs= turb_q_daily= day_spill_proportion= ;
	end;
	output;
%mend proj_to_SI;

%macro proj_day (proj);

	%let tot = _tot_Q_HS;
	%let other = _other_HS;
	%let turb = _turb_HS;
	%let tot_spill = _tot_spill_HS;
	%let dp = mcn_daily_Q_prop;
	%let force = _force;
	%let bypass = _bypass_HS ; ** 5-31 **;

	%let tot_d = _tot_Q_daily;
	%let other_d = _other_daily;
	%let turb_d = _turb_daily;
	%let spill_d = _spill_day_daily;
	%let spill_n = _spill_nite_daily;
 	%let sp_win_mon = _in_mon_sp_prop ; 

		&proj&tot_d = &proj&tot * &dp ;
		&proj&spill_d =  &dp  * (&proj&tot_spill - &proj&bypass + &proj&sp_win_mon * &proj&bypass ) ; ** 6-1 **;
		&proj&spill_n =  &dp  * (&proj&tot_spill - &proj&bypass + &proj&sp_win_mon * &proj&bypass ) ; ** 6-1 **;


%mend proj_day;
%macro project (proj, plant);

	%let tot = _tot_q_HS;
	%let other = _other_HS;
	%let turb = _turb_HS;
	%let elev = _elev_feet;
	%let tot_spill = _tot_spill_HS;
	%let force = _force;
	%let bypass = _bypass_HS ; ** 5-31 **;

	data &proj;
		set temp;
		if plant = "&plant" ;
		constant = 1;
		&proj&tot = q_out / 1000 ;
		&proj&other = q_other / 1000 ;
		&proj&turb = q_turbine  / 1000 ;
		&proj&tot_spill = q_tot_spill  / 1000 ;
		&proj&bypass = q_bypass / 1000 ;
		&proj&elev = elev_feet;
		&proj&force = force_on ;
		cal_mon = calendar_month ;
		cal_yr = calendar_yr ;
		
		keep cal_yr cal_mon &proj&tot_spill &proj&bypass &proj&tot &proj&elev &proj&force;
	run;
	proc sort;
		by cal_yr cal_mon ;
	run;
%mend project;
data within_mon_spill; *** new 06-01-2007 ;
infile "D:\userfile\ptagis\programs\COMPASS Flow Mod 70 yr test\PImotion1 and 2 2008-12-16\PI motion within month spill Dec2008 for 3.csv"
	dsd lrecl=500 pad;
/** Cal_date,Julian_day,cal_mon,
BON_in_mon_sp_prop,TDA_in_mon_sp_prop,JDA_in_mon_sp_prop,MCN_in_mon_sp_prop,
PRD_in_mon_sp_prop,IHR_in_mon_sp_prop,LGS_in_mon_sp_prop,LMN_in_mon_sp_prop,
LWG_in_mon_sp_prop,HELL_in_mon_sp_prop,DWR_in_mon_sp_prop
**/
	*** length cal_mon $8. ;
	input Cal_date : ANYDTDTM25.
		julian_day : 10. 
		cal_mon : $15.
  		BON_in_mon_sp_prop
		TDA_in_mon_sp_prop
		JDA_in_mon_sp_prop
		MCN_in_mon_sp_prop
		PRD_in_mon_sp_prop
		IHR_in_mon_sp_prop
		LGS_in_mon_sp_prop
		LMN_in_mon_sp_prop
		LWG_in_mon_sp_prop
		HELL_in_mon_sp_prop
		DWR_in_mon_sp_prop  ;

	if julian_day ne . ;

	cal_mon = substr(cal_mon, 2, 8) ;
	do cal_yr = 1995 to 2006; ** to match with MCN flow file for modulation ;
		output;
	end;
	** drop cal_mon_char cal_date;
run;
proc sort;
	by cal_yr cal_mon julian_day;
run;
proc contents;
	title2 "within_mon_spill";
run;
proc means n nmiss min mean max;
run;
options obs=100;
proc print;
run;
options obs=max;
run;

data ana;
	set sasdata.anatone_70_for_sas;
	cal_yr = CAL_YEAR    ;

cal_mon = 	"01 - JAN" ; ANA_tot_q_HS = jan_01/1000; output;	   
cal_mon = 	"02 - FEB" ; ANA_tot_q_HS = feb_02/1000; output;	   
cal_mon = 	"03 - MAR" ; ANA_tot_q_HS = mar_03/1000; output;	   
cal_mon = 	"04 - AP1" ; ANA_tot_q_HS = ap1_04/1000; output;   
cal_mon = 	"05 - AP2" ; ANA_tot_q_HS = ap2_05/1000; output;	   
cal_mon = 	"06 - MAY" ; ANA_tot_q_HS = may_06/1000; output;	   
cal_mon = 	"07 - JUN" ; ANA_tot_q_HS = jun_07/1000; output;	   
cal_mon = 	"08 - JUL" ; ANA_tot_q_HS = jul_08/1000; output;	   
cal_mon = 	"09 - AU1" ; ANA_tot_q_HS = au1_09/1000; output;	   
cal_mon = 	"10 - AU2" ; ANA_tot_q_HS = au2_10/1000; output;	   
cal_mon = 	"11 - SEP" ; ANA_tot_q_HS = sep_11/1000; output;	 
cal_mon = 	"12 - OCT" ; ANA_tot_q_HS = oct_12/1000; output;	   
cal_mon = 	"13 - NOV" ; ANA_tot_q_HS = nov_13/1000; output;	   
cal_mon = 	"14 - DEC" ; ANA_tot_q_HS = dec_14/1000; output;	   

	keep cal_yr cal_mon ANA_tot_q_HS ;


run;
proc sort;
	by cal_yr cal_mon ;
run;
proc contents;
	title2 "ANAone";
run;
proc means n nmiss min mean max ;
run;

data oro;
	set sasdata.oro_70_for_sas;
	cal_yr = CAL_YEAR    ;

cal_mon = 	"01 - JAN" ; ORO_tot_q_HS = jan_01/1000; output;	   
cal_mon = 	"02 - FEB" ; ORO_tot_q_HS = feb_02/1000; output;	   
cal_mon = 	"03 - MAR" ; ORO_tot_q_HS = mar_03/1000; output;	   
cal_mon = 	"04 - AP1" ; ORO_tot_q_HS = ap1_04/1000; output;   
cal_mon = 	"05 - AP2" ; ORO_tot_q_HS = ap2_05/1000; output;	   
cal_mon = 	"06 - MAY" ; ORO_tot_q_HS = may_06/1000; output;	   
cal_mon = 	"07 - JUN" ; ORO_tot_q_HS = jun_07/1000; output;	   
cal_mon = 	"08 - JUL" ; ORO_tot_q_HS = jul_08/1000; output;	   
cal_mon = 	"09 - AU1" ; ORO_tot_q_HS = au1_09/1000; output;	   
cal_mon = 	"10 - AU2" ; ORO_tot_q_HS = au2_10/1000; output;	   
cal_mon = 	"11 - SEP" ; ORO_tot_q_HS = sep_11/1000; output;	 
cal_mon = 	"12 - OCT" ; ORO_tot_q_HS = oct_12/1000; output;	   
cal_mon = 	"13 - NOV" ; ORO_tot_q_HS = nov_13/1000; output;	   
cal_mon = 	"14 - DEC" ; ORO_tot_q_HS = dec_14/1000; output;	
   
	keep cal_yr cal_mon ORO_tot_q_HS ;

run;
proc sort;
	by cal_yr cal_mon ;
run;
proc contents;
	title2 "orofino";
run;
proc means n nmiss min mean max ;
run;
data hell_2000;
	set sasdata.brownlee_70_for_sas;
	cal_yr = CAL_YEAR    ;

cal_mon = 	"01 - JAN" ; HELL_tot_q_2000 = jan_01/1000; output;	   
cal_mon = 	"02 - FEB" ; HELL_tot_q_2000 = feb_02/1000; output;	   
cal_mon = 	"03 - MAR" ; HELL_tot_q_2000 = mar_03/1000; output;	   
cal_mon = 	"04 - AP1" ; HELL_tot_q_2000 = ap1_04/1000; output;   
cal_mon = 	"05 - AP2" ; HELL_tot_q_2000 = ap2_05/1000; output;	   
cal_mon = 	"06 - MAY" ; HELL_tot_q_2000 = may_06/1000; output;	   
cal_mon = 	"07 - JUN" ; HELL_tot_q_2000 = jun_07/1000; output;	   
cal_mon = 	"08 - JUL" ; HELL_tot_q_2000 = jul_08/1000; output;	   
cal_mon = 	"09 - AU1" ; HELL_tot_q_2000 = au1_09/1000; output;	   
cal_mon = 	"10 - AU2" ; HELL_tot_q_2000 = au2_10/1000; output;	   
cal_mon = 	"11 - SEP" ; HELL_tot_q_2000 = sep_11/1000; output;	 
cal_mon = 	"12 - OCT" ; HELL_tot_q_2000 = oct_12/1000; output;	   
cal_mon = 	"13 - NOV" ; HELL_tot_q_2000 = nov_13/1000; output;	   
cal_mon = 	"14 - DEC" ; HELL_tot_q_2000 = dec_14/1000; output;	
   
	keep cal_yr cal_mon HELL_tot_q_2000 ;
run;
proc sort;
	by cal_yr cal_mon ;
run;
data temp;
	infile "D:\userfile\ptagis\programs\COMPASS Flow Mod 70 yr test\PImotion1 and 2 2008-12-16\PImotionSpill&Res+plus.PRT" 
		lrecl=140 pad expandtabs;

	length hydro_period  $11. temp_year $5. plant $8. t_chr $32. calendar_month $12. ;

	retain 	hydro_period  hydro_year calendar_month calendar_yr;

	input @1 mychar $140. @;
	if index (mychar, "BPA REGULATOR OUTPUT FOR") > 0 then do;
		hydro_period = substr(mychar, 25, 11) ;
		temp_year = substr (mychar, 62, 5) ;
		hydro_year = input (temp_year, 5.) ;
	end;
	calendar_month = " " ;
 
If index(hydro_period, "APRIL 1-15" ) > 0  	then calendar_month = 	"04 - AP1" ; 	   
If index(hydro_period, "APRIL16-30" ) > 0   	then calendar_month = 	"05 - AP2" ; 	   
If index(hydro_period, "AUG 1-15" ) > 0   	then calendar_month = 	"09 - AU1" ; 	   
If index(hydro_period, "AUG 16-31" ) > 0   	then calendar_month = 	"10 - AU2" ; 	   
If index(hydro_period, "DECEMBER" ) > 0   	then calendar_month = 	"14 - DEC" ; 	   
If index(hydro_period, "FEBRUARY" ) > 0   	then calendar_month = 	"02 - FEB" ; 	   
If index(hydro_period, "JANUARY" ) > 0   		then calendar_month = 	"01 - JAN" ; 	   
If index(hydro_period, "JULY" ) > 0   		then calendar_month = 	"08 - JUL" ; 	   
If index(hydro_period, "JUNE" ) > 0   		then calendar_month = 	"07 - JUN" ; 	   
If index(hydro_period, "MARCH" ) > 0   		then calendar_month = 	"03 - MAR" ; 	   
If index(hydro_period, "MAY" ) > 0   		then calendar_month = 	"06 - MAY" ; 	   
If index(hydro_period, "NOVEMBER" ) > 0   	then calendar_month = 	"13 - NOV" ; 	   
If index(hydro_period, "OCTOBER" ) > 0   		then calendar_month = 	"12 - OCT" ; 	   
If index(hydro_period, "SEPTEMBER" ) > 0   	then calendar_month = 	"11 - SEP" ; 	 

	test_str = substr(calendar_month , 1, 2) ;
	test_num = input (test_str, 2.) ;
	if test_num ge 8 then calendar_yr = hydro_year - 1;
		else calendar_yr = hydro_year;
	if calendar_yr = 1928 then calendar_yr =1998 ; *** WAS 1978;

	header = "PLANT" ; ** for header info only ;
	prd = "PRIEST" ; 
	HELL = "HELL C* " ;  
	DWR = "DWRSHK" ;    
 	LWG = "LR.GRN" ;     
 	LGS = "L GOOS" ;     
 	LMN = "LR MON" ;    
 	IHR = "ICE H" ;    
 	MCN = "MCNARY" ;   
 	JDA = "J DAY" ;   
 	TDA = "DALLES" ;   
 	BON = "BONN" ;    
array proj (11) bon tda jda mcn IHR lmn lgs LWG dwr hell prd ;
	do i = 1 to 11;
		if index(mychar, proj(i)) > 0 then do;
			plant = substr (mychar, 1, 8) ;
			t_chr = substr(mychar, 9, 4);
			plant_num = input (t_chr, 4.);
			t_chr = substr(mychar, 21, 7);
			q_out = input (t_chr, 7.);
			t_chr = substr(mychar, 28, 7);
			q_min = input(t_chr, 7.) ;
			t_chr = substr(mychar, 35, 7);
			q_forced = input (t_chr, 7.);
			t_chr = substr(mychar, 42, 7);
			q_bypass = input(t_chr, 7.) ;
			t_chr = substr(mychar, 49, 7);
			q_other = input (t_chr, 7.);
			t_chr = substr(mychar, 56, 7);
			q_overgen = input (t_chr, 7.) ;
			t_chr = substr(mychar, 91, 7 );
			elev_feet = input (t_chr, 7.) ;
			q_turbine = q_out - q_forced - q_bypass - q_overgen ; 
		*** NOTE 3-15  - q_other now in turbine flow per KAF ***;
			q_tot_spill = q_forced + q_bypass + q_overgen;
			if (q_forced > 0 or q_overgen > 0) then force_on = 1;
				else force_on = 0;
			output;
		end;
	end;
	drop header bon tda jda mcn IHR lmn lgs LWG dwr hell prd temp_year i t_chr mychar;
run;

%project (BON, BONN);
%project (PRD , PRIEST);
%project (HELL, HELL C*);
%project (DWR, DWRSHK);
%project (LWG, LR.GRN);
%project (LGS, L GOOS);
%project (LMN, LR MON);
%project (IHR, ICE H);
%project (MCN, MCNARY);
%project (JDA, J DAY);
%project (TDA, DALLES);

data hysim_monthly(label = "HYDROSIM Monthly, 2004 base case, KCFS");
	merge BON TDA JDA MCN PRD IHR LGS LMN LWG HELL DWR ana oro hell_2000;
	by cal_yr cal_mon ;
	dwr_tot_q_HS = dwr_tot_q_HS + oro_tot_q_HS;
	hell_tot_q_hs =hell_tot_q_hs  + ana_tot_q_hs - HELL_tot_q_2000 ;  ** these per RD memoFeb 1 2007;
 run;
proc means n nmiss min mean max;
	title2 "hysim_monthly";
run;
** now read real daily stuff;
** THIS IS FOR SIMULATION NOT MATCHING ;
data daily_flow;
	set sasdata.daily_q_to_sas (rename = (julian_day = jul_char));
	*** created from EXCEL with JMP ;
	julian_day = input (jul_char, 4.0);
	if julian_day <= 31 then cal_mon = "01 - JAN";
	if julian_day <= 59 and cal_mon = " " then  cal_mon = "02 - FEB";
	if julian_day <= 90 and cal_mon = " " then  cal_mon = "03 - MAR";
	if julian_day <= 105 and cal_mon = " " then  cal_mon = "04 - AP1";
	if julian_day <= 120 and cal_mon = " " then  cal_mon = "05 - AP2";
	if julian_day <= 151 and cal_mon = " " then  cal_mon = "06 - MAY";
	if julian_day <= 181 and cal_mon = " " then  cal_mon = "07 - JUN";
	if julian_day <= 212 and cal_mon = " " then  cal_mon = "08 - JUL";
	if julian_day <= 227 and cal_mon = " " then  cal_mon = "09 - AU1";
	if julian_day <= 243 and cal_mon = " " then  cal_mon = "10 - AU2";
	if julian_day <= 273 and cal_mon = " " then  cal_mon = "11 - SEP";
	if julian_day <= 304 and cal_mon = " " then  cal_mon = "12 - OCT";
	if julian_day <= 334 and cal_mon = " " then  cal_mon = "13 - NOV";
	if julian_day <= 365 and cal_mon = " " then  cal_mon = "14 - DEC";

	keep  cal_yr julian_day MCN_Actual_daily_tot_q_kcfs cal_mon ; 

run;

proc sort;
	by cal_yr cal_mon;
run;
proc summary data = daily_flow NWAY ;
	var MCN_Actual_daily_tot_q_kcfs    ;
	class cal_yr cal_mon;
	output out = mcn_mon_q_act
	mean = MCN_Actual_mean_flow;
run;

proc sort;
	by cal_yr cal_mon;
run;

data daily_flow_prop;
	merge daily_flow mcn_mon_q_act;
	by cal_yr cal_mon;
	mcn_daily_Q_prop = MCN_Actual_daily_tot_q_kcfs / MCN_Actual_mean_flow;
	if mcn_daily_Q_prop = . then do;
		put _all_;
		mcn_daily_Q_prop = 1;
	end;
run;

proc sort;
	by cal_yr cal_mon julian_day;
run;

data mcn_daily_props_actual;
	merge daily_flow_prop within_mon_spill; *** new 5-31-07 ;
	by cal_yr cal_mon julian_day;
		real_yr = cal_yr;
	if cal_yr > . and cal_mon ne " " and julian_day ne . ; 
	drop cal_yr;
	label real_yr = "Real year for real flow at MCN for COMPASS match";
	if mcn_daily_Q_prop = . then mcn_daily_Q_prop = 1;
run;
proc sort;
	by real_yr cal_mon julian_day;
run;
proc contents;
	title2 "mcn_daily_props_actual";
run;
proc means n nmiss min mean max ;
run;

************************** Below on down Uses above and KF input to do Apr-Jun match ** ;
data real_vs_cal;
	set sasdata.year_match_70_yr_07_12;
run;
proc sort;
	by real_yr cal_yr;
run;

proc sql;
	create table for_daily as
	select * from real_vs_cal, mcn_daily_props_actual 
	where real_vs_cal.real_yr = mcn_daily_props_actual.real_yr;
quit;

proc sort data = for_daily  
	(keep = cal_yr cal_mon julian_day mcn_daily_Q_prop

 
	BON_in_mon_sp_prop	
	TDA_in_mon_sp_prop	
	JDA_in_mon_sp_prop	
	MCN_in_mon_sp_prop	
	PRD_in_mon_sp_prop	
	IHR_in_mon_sp_prop	
	LGS_in_mon_sp_prop	
	LMN_in_mon_sp_prop	
	LWG_in_mon_sp_prop	
	HELL_in_mon_sp_prop	
	DWR_in_mon_sp_prop ) 
	out= for_daily_std;
	by cal_yr cal_mon julian_day;
run;
data hydsim_w_daily_like_mcn_Apr_Jun;
	merge hysim_monthly for_daily_std;
		by cal_yr cal_mon ;
	
	%proj_day (BON) ;
	%proj_day (TDA) ;
	%proj_day (JDA) ;
	%proj_day (MCN) ;
	%proj_day (PRD) ;
	%proj_day (IHR) ;
	%proj_day (LGS) ;
	%proj_day (LMN) ;
	%proj_day (LWG) ;
	%proj_day (HELL) ;
	%proj_day (DWR) ;
	format  mcn_daily_Q_prop  4.1 ;
run;
proc contents;
	title2 "merged_hysim_daily";
run;
proc means n nmiss min mean max;
run;
data trial_to_susannah;
	length project $3. ;
	retain project cal_yr julian_day daily_flow_kcfs day_spill_proportion night_spill_proportion
		monthly_elevation_feet;
	format cal_yr 4.0
		julian_day 3.0
		daily_flow_kcfs 8.2 
		day_spill_proportion 5.4
		night_spill_proportion 5.4
		monthly_elevation_feet 8.1;
	keep  project cal_yr julian_day daily_flow_kcfs day_spill_proportion night_spill_proportion
		monthly_elevation_feet spill_prob turb_q_daily ;
	set hydsim_w_daily_like_mcn_Apr_Jun;
	%proj_to_SI (BON, BON) ;
	%proj_to_SI (TDA, TDA) ;
	%proj_to_SI (JDA, JDA) ;
	%proj_to_SI (MCN, MCN) ;
	%proj_to_SI (PRD, PRD) ;
	%proj_to_SI (IHR, ICH) ;
	%proj_to_SI (LGS, LGS) ;
	%proj_to_SI (LMN, LMN) ;
	%proj_to_SI (LWG, LWG) ;
	%proj_to_SI (HELL, ANA) ;
	%proj_to_SI (DWR, PEC) ;
run;
proc sort;
	by project cal_yr julian_day;
run;
proc means n nmiss min mean max sum;
	by project ;
	title2 "2004_to_susannah" ;
run;
proc freq;
	tables project;
run;
options missing = " ";
proc export data=trial_to_susannah (drop = spill_prob turb_q_daily ) replace
	dbms = csv
	outfile = "D:\userfile\ptagis\programs\COMPASS Flow Mod 70 yr test\PImotion1 and 2 2008-12-16\PImotion3_2008_12_17.csv";
run;

data spill_probs;
	set  trial_to_susannah;
	if spill_prob > 0 ;
run;
proc print;
	title2 "spill problems";
run;