% ------------actionLabels-------------------------------------------
% CART TELEMETRY PLOTTER
% Columns: Time, Mode, Speed, PinCmd, Action, xValue, yValue, PatternState
% -------------------------------------------------------
clear; clc; close all;

% -------------------------------------------------------
% LOAD DATA
% -------------------------------------------------------
filename = '20260609_0257 CARTdata-test.txt';  % change to your file name

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
% PinCmd: safely convert binary strings to decimal, guard all invalid values
pinCmdNum = zeros(height(T), 1);
for i = 1:height(T)
    if ismissing(T.PinCmd(i))
        pinCmdNum(i) = 0;
        continue
    end
    val = char(T.PinCmd(i));
    if ~isempty(val) && all(val == '0' | val == '1')
        pinCmdNum(i) = bin2dec(val);
    else
        pinCmdNum(i) = 0;
    end
end

% -------------------------------------------------------
% PLOT
% -------------------------------------------------------
figure('Name', 'Cart Telemetry', 'NumberTitle', 'off', 'Position', [100, 100, 1200, 1100]);

left  = 0.08;
width = 0.88;
gap   = 0.015;

% Heights: plot 4 is 2x, all others are 1x
% 6 normal plots + 1 double = 8 units total
unit = (1 - 2*gap*8) / 8;  % each "unit" of height
h = [unit, unit, unit, 2*unit, unit, unit, unit];  % heights for plots 1-7

% Compute bottom positions (top to bottom, so plot 1 is highest)
tops = zeros(1,7);
tops(1) = 1 - 0.04 - h(1);
for k = 2:7
    tops(k) = tops(k-1) - gap - h(k);
end

% --- 1. Mode ---
axes('Position', [left, tops(1), width, h(1)]);
stairs(time, modeNum, 'b', 'LineWidth', 1.2);
yticks([0 1]); yticklabels({'JOYSTICK','AI\_CAM'});
ylabel('Mode'); ylim([-0.2 1.2]);
title('Mode'); grid on;

% --- 2. Speed ---
axes('Position', [left, tops(2), width, h(2)]);
plot(time, T.Speed, 'r', 'LineWidth', 1.2);
ylabel('Speed');
title('Speed (0-127)'); grid on;

% --- 3. PinCmd ---
axes('Position', [left, tops(3), width, h(3)]);
stairs(time, pinCmdNum, 'm', 'LineWidth', 1.2);
yticks(0:5);
yticklabels({'000','001','010','011','100','101'});
ylabel('PinCmd'); ylim([-0.5 5.5]);
title('Pi Pin Command (binary)'); grid on;

% --- 4. Action (2x tall) ---
axes('Position', [left, tops(4), width, h(4)]);
stairs(time, actionNum, 'k', 'LineWidth', 1.2);
yticks(1:length(actionLabels));
yticklabels(actionLabels);
ylabel('Action');
title('Cart Action'); grid on;

% --- 5. xValue ---
axes('Position', [left, tops(5), width, h(5)]);
plot(time, T.xValue, 'Color', [0 0.6 0], 'LineWidth', 1.2);
yline(100, '--r', 'xValue=100 (switch)');
yline(400, '--', 'Color', [0.5 0.5 0.5]);
yline(600, '--', 'Color', [0.5 0.5 0.5]);
yline(900, '--b', 'xValue=900 (fast fwd)');
ylabel('xValue'); ylim([0 1023]);
title('Joystick X'); grid on;

% --- 6. yValue ---
axes('Position', [left, tops(6), width, h(6)]);
plot(time, T.yValue, 'Color', [0.8 0.4 0], 'LineWidth', 1.2);
yline(400, '--r', 'yValue=400 (turn left)');
yline(600, '--b', 'yValue=600 (turn right)');
ylabel('yValue'); ylim([0 1023]);
title('Joystick Y'); grid on;

% --- 7. PatternState ---
axes('Position', [left, tops(7), width, h(7)]);
stairs(time, T.PatternState, 'Color', [0.5 0 0.5], 'LineWidth', 1.2);
yticks([-1 0 1]); yticklabels({'uninit','STOP','BACK'});
ylabel('PatternState'); ylim([-1.5 1.5]);
title('Pattern Detector State'); grid on;
xlabel('Time (s)');

% Shared formatting
sgtitle('Cart Telemetry Log', 'FontSize', 14, 'FontWeight', 'bold');
axList = findall(gcf, 'Type', 'axes');
linkaxes(axList, 'x');

% -------------------------------------------------------
% SHARED FORMATTING
% -------------------------------------------------------
sgtitle('Cart Telemetry Log', 'FontSize', 14, 'FontWeight', 'bold');

% Link all x-axes so zooming one zooms all
axList = findall(gcf, 'Type', 'axes');
linkaxes(axList, 'x');