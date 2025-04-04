%% Parameters for Power Factor Correction for CCM Boost Converter Example
%
% This example shows how to correct the power factor using a PFC 
% pre-converter. This technique is useful when non-linear impedances, 
% such as Switch Mode Power Supplies, are connected to an AC grid. As the 
% current flowing through the inductor is never zero during the switching 
% cycle, the boost converter operates in Continuous Conduction Mode (CCM). 
% The inductor current and the output voltage profiles are controlled using 
% simple integral control. During start up, the reference output voltage is 
% ramped up to the desired voltage.

% Copyright 2018-2023 The MathWorks, Inc.

%% Design Parameters
Line_Voltage_DC     = 24;                         %DC line voltage for AC sweep                           
Line_Voltage_Peak   = 24*sqrt(2);                 %  Input Voltage Peak to the Full Bridge Rectifier [V]
f_line              = 50;                         %  Power line frequency [Hz]
Voref               = 40;                         %  Desired Output Voltage from PFC Preconverter [V]
Power               = 40;                         %  Maximum steady state power capability [W]
R                   = 40;                         %  Arbitrary resistive load [Ohms]
del_V               = 4;                          %  Peak-Peak Output Voltage Ripple [V]
IndCurrRipple       = 20;                         %  Inductor Current Ripple [%]
Conv_efficiency     = 90;                         %  Nominal efficiency of the preconverter [%]
t_holdup            = 10e-3;                      %  Minimum hold up time [sec]
v_holdup            = 34;                         %  Minimum voltage at hold up time [V]
f_sw                = 50e3;                       %  MOSFET Switching Frequency [Hz]                    

%% Converter Design

Duty = (Voref-Line_Voltage_Peak)/Voref;
DC_Duty = 1 - Line_Voltage_DC / Voref;
I_in_peakmax = 2*(Power/(Conv_efficiency*0.01))/Line_Voltage_Peak;

%% Filter Inductance

del_I = (IndCurrRipple/100)*I_in_peakmax;
L = (Line_Voltage_Peak*Duty)/(f_sw*del_I);

%% Filter Capacitance

C = max((2*Power*t_holdup)/((Voref)^2-v_holdup^2),Power/(2*pi*f_line*del_V*Voref));

