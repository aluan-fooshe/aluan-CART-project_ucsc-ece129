% -------------------------------------------------------
% CART TELEMETRY PLOTTER
% Columns: Time, Mode, Speed, PinCmd, Action, xValue, yValue, PatternState
% -------------------------------------------------------
clear; clc; close all;

% -------------------------------------------------------
% LOAD DATA
% -------------------------------------------------------
filename = '20260608_2000-CARTdata-test.txt';  % change to your file name

opts = detectImportOptions(filename, 'Delimiter', ',');
opts.VariableNames = {'Time','Mode','Speed','PinCmd','Action','xValue','yValue','PatternState'};
opts = setvartype(opts, {'Mode','PinCmd','Action'}, 'string');
opts = setvartype(opts, {'Time','Speed','xValue','yValue','PatternState'}, 'double');
T = readtable(filename, opts);

time = T.Time;

% -------------------------------------------------------
% ENCODE CATEGORICAL COLUMNS AS NUMBERS FOR PLOTTING
% -------------------------------------------------------

% Mode: JOYSTICK = 0, AI_CAM = 1
modeNum = double(T.Mode == "AI_CAM");

% Action: encode each unique action as a number
[actionLabels, ~, actionNum] = unique(T.Action);

% PinCmd: already binary string like "000", "010" — convert to decimal
% guard against <missing> values (e.g. from NO_SIGNAL rows)
pinCmdNum = zeros(height(T), 1);
for i = 1:height(T)
    if ~ismissing(T.PinCmd(i))
        pinCmdNum(i) = bin2dec(char(T.PinCmd(i)));
    else
        pinCmdNum(i) = 0;  % default to 000 for missing/NO_SIGNAL rows
    end
end

% -------------------------------------------------------
% PLOT
% -------------------------------------------------------
figure('Name', 'Cart Telemetry', 'NumberTitle', 'off');
numPlots = 7;

% --- 1. Mode ---
subplot(numPlots, 1, 1);
stairs(time, modeNum, 'b', 'LineWidth', 1.2);
yticks([0 1]); yticklabels({'JOYSTICK','AI\_CAM'});
ylabel('Mode'); ylim([-0.2 1.2]);
title('Mode'); grid on;

% --- 2. Speed ---
subplot(numPlots, 1, 2);
plot(time, T.Speed, 'r', 'LineWidth', 1.2);
ylabel('Speed');
title('Speed (0-127)'); grid on;

% --- 3. PinCmd (binary -> decimal) ---
subplot(numPlots, 1, 3);
stairs(time, pinCmdNum, 'm', 'LineWidth', 1.2);
yticks(0:5);
yticklabels({'000','001','010','011','100','101'});
ylabel('PinCmd'); ylim([-0.5 5.5]);
title('Pi Pin Command (binary)'); grid on;

% --- 4. Action ---
subplot(numPlots, 1, 4);
stairs(time, actionNum, 'k', 'LineWidth', 1.2);
yticks(1:length(actionLabels));
yticklabels(actionLabels);
ylabel('Action');
title('Cart Action'); grid on;

% --- 5. xValue (joystick X) ---
subplot(numPlots, 1, 5);
plot(time, T.xValue, 'Color', [0 0.6 0], 'LineWidth', 1.2);
yline(100, '--r', 'xValue=100 (switch)');
yline(400, '--', 'Color', [0.5 0.5 0.5]);
yline(600, '--', 'Color', [0.5 0.5 0.5]);
yline(900, '--b', 'xValue=900 (fast fwd)');
ylabel('xValue'); ylim([0 1023]);
title('Joystick X'); grid on;

% --- 6. yValue (joystick Y) ---
subplot(numPlots, 1, 6);
plot(time, T.yValue, 'Color', [0.8 0.4 0], 'LineWidth', 1.2);
yline(400, '--r', 'yValue=400 (turn left)');
yline(600, '--b', 'yValue=600 (turn right)');
ylabel('yValue'); ylim([0 1023]);
title('Joystick Y'); grid on;

% --- 7. PatternState ---
subplot(numPlots, 1, 7);
stairs(time, T.PatternState, 'Color', [0.5 0 0.5], 'LineWidth', 1.2);
yticks([-1 0 1]); yticklabels({'uninit','STOP','BACK'});
ylabel('PatternState'); ylim([-1.5 1.5]);
title('Pattern Detector State'); grid on;
xlabel('Time (s)');

% -------------------------------------------------------
% SHARED FORMATTING
% -------------------------------------------------------
sgtitle('Cart Telemetry Log', 'FontSize', 14, 'FontWeight', 'bold');

% Link all x-axes so zooming one zooms all
axList = findall(gcf, 'Type', 'axes');
linkaxes(axList, 'x');