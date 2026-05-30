%% MicroMotionStudioDATAplot.m 
% Basic Micro Motion Studio data file P-plotter
% Reads motorsLog.csv and plots motor telemetry vs. time

clear; clc; close all;

%% --- Load Data ---
data = readtable('FRONTmotorsLog.csv');

t = data.Time;  % x-axis: time in seconds

%% --- Define subplots layout ---
figure('Name', 'FRONT Motors Log', 'NumberTitle', 'off', ...
       'Units', 'normalized', 'Position', [0.05 0.05 0.9 0.88]);

% Colour scheme
c1 = [0.00 0.45 0.74];  % blue   - M1
c2 = [0.85 0.33 0.10];  % orange - M2

%% 1 - Speed (encoder counts/s or RPM depending on your setup)
subplot(5,2,[1 2]);
plot(t, data.M1Speed, 'Color', c1, 'LineWidth', 1.2); hold on;
plot(t, data.M2Speed, 'Color', c2, 'LineWidth', 1.2);
ylabel('Speed (counts/s)');
title('Motor Speed');
legend('M1 Speed', 'M2 Speed', 'Location', 'best');
grid on; xlim([t(1) t(end)]);

%% 2 - Current
subplot(5,2,[3 4]);
plot(t, data.M1Current, 'Color', c1, 'LineWidth', 1.2); hold on;
plot(t, data.M2Current, 'Color', c2, 'LineWidth', 1.2);
ylabel('Current (A)');
title('Motor Current');
legend('M1 Current', 'M2 Current', 'Location', 'best');
grid on; xlim([t(1) t(end)]);

%% 3 - Temperature
subplot(5,2,[5 6]);
plot(t, data.Temp1, 'Color', [0.93 0.69 0.13], 'LineWidth', 1.4); hold on;
plot(t, data.Temp2, 'Color', [0.49 0.18 0.56], 'LineWidth', 1.4);
ylabel('Temp (°C)');
title('Temperature');
legend('Temp1', 'Temp2', 'Location', 'best');
grid on; xlim([t(1) t(end)]);

%% 4 - Main Battery Voltage
subplot(5,2,[7 8]);
plot(t, data.MBatt, 'Color', [0.47 0.67 0.19], 'LineWidth', 1.4);
ylabel('Voltage (V)');
title('Main Battery Voltage');
legend('MBatt', 'Location', 'best');
grid on; xlim([t(1) t(end)]);

%% 5 - Logic Battery (flag / voltage)
subplot(5,2,[9 10]);
plot(t, data.LBatt, 'Color', [0.30 0.75 0.93], 'LineWidth', 1.2);
ylabel('LBatt');
title('Logic Battery');
legend('LBatt', 'Location', 'best');
grid on; xlim([t(1) t(end)]);

%% Shared x-label
xlabel('Time (s)');

%% Link x-axes so zooming syncs all panels
allAxes = findall(gcf, 'Type', 'axes');
linkaxes(allAxes, 'x');

sgtitle('FRONT Motors Telemetry Log', 'FontSize', 14, 'FontWeight', 'bold');