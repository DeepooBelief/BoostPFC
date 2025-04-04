clear
close all
% State-space average model of a Boost Converter 
% u=[delta];  x=[i_L; v_C];  y=[v_O] 


f_sw = 50e3; % switching freq
Ts    = 1/(30*f_sw);        %  Sampling time for the plant [sec]
Tsc   = 1/(f_sw);         %  Sampling time for the controller [sec]

% % % Component Parameters 
L=330e-6;  r_L=0.11;  Cap=3300e-6;  r_C=0.0407;  R=40; 

% Circuit Conditions 
V_I = 24; %Input voltage, rms  
V_O = 40; %Output voltage
Delta=1-V_I/V_O;

% L=30e-3;  r_L=0.05;  C=200e-6;  r_C=0.02;  R=24; 
% 
% % Circuit Conditions 
% V_I=12;  Delta=0.5; 

% First form models of the two states 
A_on = [-r_L/L,  0;  0,  -1/Cap/(R+r_C)]; 
B_on = [1/L;  0]; 
C_on = [0,  R/(R+r_C)]; 
 
A_off = [-r_L/L-r_C*R/L/(R+r_C),  -R/L/(R+r_C);  R/Cap/(R+r_C),  -1/Cap/(R+r_C)]; 
B_off = [1/L;  0]; 
C_off = [r_C*R/(R+r_C),  R/(R+r_C)]; 
 
% Average the two models 
% Operating point model 
A = Delta*A_on + (1-Delta)*A_off; 
B = Delta*B_on + (1-Delta)*B_off; 
C = Delta*C_on + (1-Delta)*C_off; 
U=V_I 
X = -inv(A)*B*U 
Y = C*X 
 
%Small-signal model 
E = (A_on-A_off)*X + (B_on-B_off)*U; 
F = (C_on-C_off)*X; 
boost_ss = ss(A,E,C,F);

% get the transfer function
s = tf('s');
Hx = (s*eye(size(A)) - A) \ E % Hx(s) = (sI - A)^(-1) * E
Hi = Hx(2);

f_z = 5;
f_p = 25;
k = 0.4;
Gv = k * tf([1/(2*pi*f_z),1], [1/(2*pi*f_p),1,0]);
figure;
margin(Gv*Hi);
% grid on;
% title('Compensated voltage loop bode plot');
% figure;
% pzmap(Gv*Hi);
% title('Compensated voltage loop pole-zero map');
% 
figure;
margin(feedback(Gv*Hi,1)) % closed-loop voltage loop bode plot, asume current loop is fast enough
% title('Closed-loop voltage loop bode plot');
Gd_v = c2d(Gv, 1/f_sw, 'tustin') % assume voltage control frequency is the same as switching frequency
[num_v, den_v] = tfdata(Gd_v, 'v');
a_v = den_v(end);
A = [1, 1, 1;
     -(1+den_v(3)),  -den_v(3), -2;
     den_v(3), 0, 1];
B = [num_v(1); num_v(2); num_v(3)];
X_pid = A \ B; % solve for PID gains
Kp_v = X_pid(1)
Ki_v = X_pid(2)
Kd_v = X_pid(3)

ACDC = 0; %var for simulink