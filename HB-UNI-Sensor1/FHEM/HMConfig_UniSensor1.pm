package main;

use strict;
use warnings;

# device definition
$HMConfig::culHmModel{'F103'} = {name => 'HB-UNI-Sensor1', st => 'THPLSensor', cyc => '00:10', rxt => 'l:w:c:f', lst  => 'p',   chn  => '',};

$HMConfig::culHmRegDefine{'lowBatLimit'}  = {a=>18.0,s=>1.0,l=>0,min=>1.0 ,max=>5     ,c=>'',f=>10,u=>'V',  d=>0,t=>'Low batterie limit, step 0.1 V.'};
$HMConfig::culHmRegDefine{'altitude'}     = {a=>34.0,s=>2.0,l=>0,min=>0   ,max=>10000 ,c=>'',f=>'',u=>'m'  ,d=>0,t=>'Altitude for calculate air pressure at see level in meter.'};

# Register model mapping
$HMConfig::culHmRegModel{'HB-UNI-Sensor1'} = {
	'burstRx'         => 1,
	'lowBatLimit'     => 1,
	'ledMode'         => 1,
	'transmDevTryMax' => 1,
	'altitude'        => 1
};

# subtype channel mapping
$HMConfig::culHmSubTypeSets{'THPLSensor'}    = {
	'peerChan'       => '0 <actChn> ... single [set|unset] [actor|remote|both]',
	'fwUpdate'       => '<filename> <bootTime> ...',
	'getSerial'      => '',
	'getVersion'     => '',
	'statusRequest'  => '',
	'burstXmit'      => ''
};

# Subtype spezific funtions
sub CUL_HM_ParseTHPLSensor(@){
	
	my ($mFlg, $frameType, $src, $dst, $msgData, $targetDevIO) = @_;
	
	my $shash = CUL_HM_id2Hash($src);                                           #sourcehash - will be modified to channel entity
	my @events = ();

	# WEATHER_EVENT
	if ($frameType eq '70'){
		my $name = $shash->{NAME};
		my $chn = '01';

		my ($dTempBat, $pressure, $humidity, $brightness, $batVoltage) = map{hex($_)} unpack ('A4A4A2A2A4', $msgData);

		# temperature
		my $temperature =  $dTempBat & 0x7fff;
		$temperature = ($temperature &0x4000) ? $temperature - 0x8000 : $temperature; 
		$temperature = sprintf('%0.1f', $temperature / 10);

		my $stateMsg = 'state:T: ' . $temperature;
		push (@events, [$shash, 1, 'temperature:' . $temperature]);

		# battery state
		push (@events, [$shash, 1, 'battery:' . ($dTempBat & 0x8000 ? 'low' : 'ok')]);

		# battery voltage
		$batVoltage = sprintf('%.2f', (($batVoltage + 0.00) / 1000));
		push (@events, [$shash, 1, 'batVoltage:' . $batVoltage]);

		# humidity
		if ($humidity)                 {
			$stateMsg .= ' H: ' . $humidity;
			push (@events, [$shash, 1, 'humidity:' . $humidity]);
		}
		
		# brightness
		$stateMsg .= ' B: ' . $brightness;
		push (@events, [$shash, 1, 'brightness:' . $brightness]);
		
		# air pressure
		if ($pressure) {
			my $pressureTxt = sprintf('%.1f', $pressure);
			$stateMsg .= ' P: '    . $pressureTxt;
			push (@events, [$shash, 1, 'pressure:'    . $pressureTxt]);

			my ($rAltitude) = split(' ', ReadingsVal($name, 'R-altitude', 0));
			my $altitude = AttrVal('global', 'altitude', -9999);
			my $pressureNN = ($altitude > -9999 && $rAltitude == 0) ? sprintf('%.1f', ($pressure + ($altitude / 8.5))) : 0;
			if ($pressureNN) {
				$stateMsg .= ' P-NN: ' . $pressureNN;
				push (@events, [$shash, 1, 'pressure-nn:' . $pressureNN]);
			}
		}

		push (@events, [$shash, 1, $stateMsg]);
	}

	return @events;
}

1;
