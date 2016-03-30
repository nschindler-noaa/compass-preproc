
###############################################################
# "Final" Draft 2008 BiOp Study
# 12.8 =~ 65 2 Tier transport (masquerading as 3 Tier)
# 20 March 2008
#
# used for Draft 2007 BiOp Study
# 14 April 2008
###############################################################

$rules{'LWG'}{'2008biopfinal'}{'transport:start_date'} = "93:111:111"; # April 3, April 21
$rules{'LWG'}{'2008biopfinal'}{'transport:range_lo_maf'} = "12.8";
$rules{'LWG'}{'2008biopfinal'}{'transport:range_hi_maf'} = "14.6";
$rules{'LWG'}{'2008biopfinal'}{'transport:flow_dam'} = "LWG";

$rules{'LGS'}{'2008biopfinal'}{'transport:start_date'} = "93:111:111"; # April 3, April 21
$rules{'LGS'}{'2008biopfinal'}{'transport:range_lo_maf'} = "12.8";
$rules{'LGS'}{'2008biopfinal'}{'transport:range_hi_maf'} = "14.6";
$rules{'LGS'}{'2008biopfinal'}{'transport:flow_dam'} = "LWG";

$rules{'LMN'}{'2008biopfinal'}{'transport:start_date'} = "93:111:111"; # April 3, April 21
$rules{'LMN'}{'2008biopfinal'}{'transport:range_lo_maf'} = "12.8";
$rules{'LMN'}{'2008biopfinal'}{'transport:range_hi_maf'} = "14.6";
$rules{'LMN'}{'2008biopfinal'}{'transport:flow_dam'} = "LWG";


###############################################################
# 2004 UPA
# May 2007 analysis
###############################################################

$rules{'LWG'}{'base2004-UPA'}{'transport:start_date'} = "93:110:110";
$rules{'LWG'}{'base2004-UPA'}{'transport:max_start'} = "93:110:";
$rules{'LWG'}{'base2004-UPA'}{'transport:range_lo'} = "70";
$rules{'LWG'}{'base2004-UPA'}{'transport:range_hi'} = "85";
$rules{'LWG'}{'base2004-UPA'}{'transport:range_lo_maf'} = "13.001";
$rules{'LWG'}{'base2004-UPA'}{'transport:range_hi_maf'} = "14";
$rules{'LWG'}{'base2004-UPA'}{'transport:flow_dam'} = "LWG";

$rules{'LGS'}{'base2004-UPA'}{'transport:start_date'} = "93:110:110";
$rules{'LGS'}{'base2004-UPA'}{'transport:max_start'} = "93:110:";
$rules{'LGS'}{'base2004-UPA'}{'transport:range_lo'} = "70";
$rules{'LGS'}{'base2004-UPA'}{'transport:range_hi'} = "85";
$rules{'LGS'}{'base2004-UPA'}{'transport:range_lo_maf'} = "13.001";
$rules{'LGS'}{'base2004-UPA'}{'transport:range_hi_maf'} = "14";
$rules{'LGS'}{'base2004-UPA'}{'transport:flow_dam'} = "LWG";

$rules{'LMN'}{'base2004-UPA'}{'transport:start_date'} = "93:110:110";
$rules{'LMN'}{'base2004-UPA'}{'transport:max_start'} = "93:110:";
$rules{'LMN'}{'base2004-UPA'}{'transport:range_lo'} = "70";
$rules{'LMN'}{'base2004-UPA'}{'transport:range_hi'} = "85";
$rules{'LMN'}{'base2004-UPA'}{'transport:range_lo_maf'} = "13.001";
$rules{'LMN'}{'base2004-UPA'}{'transport:range_hi_maf'} = "14";
$rules{'LMN'}{'base2004-UPA'}{'transport:flow_dam'} = "LWG";

$rules{'WEL'}{'base2004-UPA'}{'planned_spill_night'} = ['106:151:0.065', '152:181:0', '182:212:0.065', '213:227:0.025', '228:365:0', '1:105:0'];
$rules{'WEL'}{'base2004-UPA'}{'planned_spill_day'} = ['106:151:0.065', '152:181:0', '182:212:0.065', '213:227:0.025', '228:365:0', '1:105:0'];
$rules{'RRH'}{'base2004-UPA'}{'planned_spill_night'} = ['91:105:0.01', '106:120:0.15', '121:151:0.21', '152:181:0.13', '182:212:0.15', '213:227:0.15', '228:365:0', '1:90:0'];
$rules{'RRH'}{'base2004-UPA'}{'planned_spill_day'} = ['91:105:0.01', '106:120:0.15', '121:151:0.21', '152:181:0.13', '182:212:0.15', '213:227:0.15', '228:365:0', '1:90:0'];
$rules{'RIS'}{'base2004-UPA'}{'planned_spill_night'} = ['91:105:0.01', '106:120:0.20', '121:151:0.20', '152:181:0.20', '182:212:0.20', '213:227:0.20', '228:365:0', '1:90:0'];
$rules{'RIS'}{'base2004-UPA'}{'planned_spill_day'} = ['91:105:0.01', '106:120:0.20', '121:151:0.20', '152:181:0.20', '182:212:0.20', '213:227:0.20', '228:365:0', '1:90:0'];
$rules{'WAN'}{'base2004-UPA'}{'planned_spill_night'} = ['1:105:0', '106:120:0.43', '121:151:0.43', '152:181:0.46', '182:212:0.49', '213:227:0.49', '228:243:0.49', '244:365:0'];
$rules{'WAN'}{'base2004-UPA'}{'planned_spill_day'} = ['1:105:0', '106:120:0.43', '121:151:0.43', '152:181:0.46', '182:212:0.49', '213:227:0.49', '228:243:0.49', '244:365:0'];
$rules{'PRD'}{'base2004-UPA'}{'planned_spill_night'} = ['1:105:0', '106:120:0.61', '121:151:0.61', '152:181:0.50', '182:212:0.39', '213:227:0.39', '228:243:0.39', '244:365:0'];
$rules{'PRD'}{'base2004-UPA'}{'planned_spill_day'} = ['1:105:0', '106:120:0.61', '121:151:0.61', '152:181:0.50', '182:212:0.39', '213:227:0.39', '228:243:0.39', '244:365:0'];

$rules{'PRD'}{'base2004-UPA'}{'elev'} = ['1:365:488']; # columbia.desc 
$rules{'CHJ'}{'base2004-UPA'}{'elev'} = ['1:365:956']; # columbia.desc 
$rules{'WEL'}{'base2004-UPA'}{'elev'} = ['1:365:781']; # columbia.desc 
$rules{'RRH'}{'base2004-UPA'}{'elev'} = ['1:365:707.4']; # columbia.desc 
$rules{'RIS'}{'base2004-UPA'}{'elev'} = ['1:365:614']; # columbia.desc 
$rules{'WAN'}{'base2004-UPA'}{'elev'} = ['1:365:572.5']; # columbia.desc 
$rules{'TDA'}{'base2004-UPA'}{'elev'} = ['1:365:160']; # columbia.desc & NOAA 

###############################################################
# OTHER variables

# names for writing flow.forecast file
our %loc_list = (
	'Columbia_Headwater', 'CHJ',
	'Deschutes_Headwater', 'TDA',
	'Snake_Headwater', 'ANA', 
	'Clearwater_Headwater', 'PEC', 
	'North_Fork_Clearwater_Headwater', 'PEC', 
	'Middle_Fork_Clearwater_Headwater', 'PEC', 
	'Salmon_Headwater', 'ANA', 
	'Wenatchee_Headwater', 'RIS',
	'Methow_Headwater', 'RRH', 
	'Bonneville_Dam', 'BON',
	'The_Dalles_Dam', 'TDA',
	'John_Day_Dam', 'JDA',
	'McNary_Dam', 'MCN',
	'Priest_Rapids_Dam', 'PRD',
	'Wanapum_Dam', 'WAN',
	'Rock_Island_Dam', 'RIS',
	'Rocky_Reach_Dam', 'RRH',
	'Wells_Dam', 'WEL',
	'Chief_Joseph_Dam', 'CHJ',
	'Ice_Harbor_Dam', 'IHR',
	'Lower_Monumental_Dam', 'LMN',
	'Little_Goose_Dam', 'LGS',
	'Lower_Granite_Dam', 'LWG',
	'Dworshak_Dam', 'DWR',
);

our %dflowMap = (
	'BON', 'Bonneville Dam',
	'TDA', 'The Dalles Dam',
	'JDA', 'John Day Dam',
	'MCN', 'McNary Dam',
	'PRD', 'Priest Rapids Dam',
	'WAN', 'Wanapum Dam',
	'RIS', 'Rock Island Dam',
	'RRH', 'Rocky Reach Dam',
	'WEL', 'Wells Dam',
	'CHJ', 'Chief Joseph Dam',
	'IHR', 'Ice Harbor Dam',
	'LMN', 'Lower Monumental Dam',
	'LGS', 'Little Goose Dam',
	'LWG', 'Lower Granite Dam',
	'DWR', 'Dworshak Dam',
	'ANA', 'Anatone USGS',
	'PEC', 'Peck USGS',
);

#CHJ does not have a pool
our %poolMap = (
	'BON', 'Bonneville_Pool',
	'TDA', 'The_Dalles_Pool',
	'JDA', 'John_Day_Pool',
	'MCN', 'McNary_Pool',
	'PRD', 'Priest_Rapids_Pool',
	'WAN', 'Wanapum_Pool',
	'RIS', 'Rock_Island_Pool',
	'RRH', 'Rocky_Reach_Pool',
	'WEL', 'Wells_Pool',
	'IHR', 'Ice_Harbor_Pool',
	'LMN', 'Lower_Monumental_Pool',
	'LGS', 'Little_Goose_Pool',
	'LWG', 'Lower_Granite_Pool',
);

our %months = (
	'Apr', ['91:120:30'],
	'May', ['121:151:31'],
	'Jun', ['152:181:30'],
	'Jul', ['182:212:31'],
	'Aug', ['213:243:31'],
	'Sep', ['244:273:30'],
	'Oct', ['274:304:31'],
	'Nov', ['305:334:30'],
	'Dec', ['335:365:31'],
	'Jan', ['1:31:31'],
	'Feb', ['32:59:28'],
	'Mar', ['60:90:31'],
);

our %monthsPeriods = (
	'Ap1', ['91:105:15'],
	'Ap2', ['106:120:15'],
	'May', ['121:151:31'],
	'Jun', ['152:181:30'],
	'Jul', ['182:212:31'],
	'Au1', ['213:227:15'],
	'Au2', ['228:243:16'],
	'Sep', ['244:273:30'],
	'Oct', ['274:304:31'],
	'Nov', ['305:334:30'],
	'Dec', ['335:365:31'],
	'Jan', ['1:31:31'],
	'Feb', ['32:59:28'],
	'Mar', ['60:90:31'],
);

our @opsParams = qw(planned_spill_night planned_spill_day); 
our @ruleParams = qw(planned_spill_night planned_spill_day elev flowmax flowmaxspill minflow maxspill ph_cap ph_avail); 
our @loc_list = qw(CHJ IHR LMN LGS LWG BON TDA JDA MCN PRD WAN RRH RIS WEL ANA PEC); 
# names for writing compass files
our @dams = qw(BON TDA JDA MCN PRD WAN RIS RRH WEL CHJ IHR LMN LGS LWG);

our @river_list = qw(DWR);
push(@river_list,@dams);
our @hist_list = qw(DWR);
push(@hist_list,@dams);
our @spill_list = qw(BON IHR JDA LGS LWG LMN MCQW PRD WAN RRH RIS WEL TDA CHJ DWQI);
our @trans_spring_list = qw(LWG LGS LMN);
our @trans_list = qw(LWG LGS LMN MCN);
#                   ANATONE  PECK
our @usgs_list = qw(13334300 13341050); 

#
our @controlDams = qw(BON TDA JDA MCN IHR LMN LGS LWG);
our @controlPeriods = qw(Jan Feb Mar Ap1 Ap2 May Jun Jul Au1 Au2 Sep Oct Nov Dec);
our @controlSpillPeriods = qw(Ap1 Ap2 May Jun);
