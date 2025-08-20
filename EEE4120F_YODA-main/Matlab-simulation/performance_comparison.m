% Define the implementations and data sizes
implementations = {'C Sequential', 'C MPI 2', 'C MPI 4', 'C MPI 8', 'C MPI 16', 'C MPI 32', 'FPGA'};
dataSizes = [10, 100, 1000, 1000000, 10000000];
dataLabels = {'10 bytes', '100 bytes', '1,000 bytes', '1,000,000 bytes', '10,000,000 bytes'};

% Selected implementations for line graphs (indices in the implementations array)
selected_impls = [1, 2, 4, 6, 7]; % C Sequential, C MPI 2, C MPI 8, C MPI 32, FPGA
selected_impl_names = implementations(selected_impls);

% MPI process counts for efficiency calculation
mpi_processes = [1, 2, 4, 8, 16, 32]; % Including sequential as 1 process
mpi_impl_names = {'C Sequential', 'C MPI 2', 'C MPI 4', 'C MPI 8', 'C MPI 16', 'C MPI 32'};

% Execution time in nanoseconds
execTime = [
    % C Sequential
    72, 454, 4174, 4370000, 42310000;
    % C MPI 2
    39.3, 239, 2099, 2170000, 20898000;
    % C MPI 4
    53, 153, 1091, 1137000, 11266000;
    % C MPI 8
    45, 119, 696, 877000, 7983000;
    % C MPI 16
    35, 74, 389, 380000, 3926000;
    % C MPI 32
    56, 65, 237, 175000, 1954000;
    % FPGA
    45, 315, 3017, 3000015, 30000015
];

% Clock cycles per byte
cyclesPerByte = [
    % C Sequential
    19.80, 13.46, 12.28, 12.50, 12.23;
    % C MPI 2
    11.80, 7.16, 6.15, 6.21, 5.98;
    % C MPI 4
    16.00, 4.60, 3.20, 3.25, 3.22;
    % C MPI 8
    13.40, 3.57, 2.04, 2.51, 2.28;
    % C MPI 16
    10.50, 2.22, 1.14, 1.09, 1.12;
    % C MPI 32
    16.80, 1.94, 0.70, 0.50, 0.56;
    % FPGA
    1.00, 1.00, 1.00, 1.00, 1.00  % Assuming 1 cycle/byte for FPGA as indicated
];

% Calculate speedup relative to C Sequential (for execution time)
speedup_time = zeros(size(execTime));
for i = 1:size(execTime, 2)
    speedup_time(:, i) = execTime(1, i) ./ execTime(:, i);
end

% Calculate speedup relative to C Sequential (for cycles per byte)
speedup_cycles = zeros(size(cyclesPerByte));
for i = 1:size(cyclesPerByte, 2)
    speedup_cycles(:, i) = cyclesPerByte(1, i) ./ cyclesPerByte(:, i);
end

% Calculate parallel efficiency for MPI implementations
% Efficiency = (Speedup / Number of Processes) * 100%
mpi_speedup_avg = mean(speedup_time(1:6, :), 2); % Average speedup across data sizes for MPI implementations
parallel_efficiency = (mpi_speedup_avg ./ mpi_processes') * 100;

% Create figure for execution time
figure(1);
set(gcf, 'Position', [100, 100, 1200, 800]);

% Plot execution time for each data size
for i = 1:length(dataSizes)
    subplot(2, 3, i);
    bar(execTime(:, i), 'FaceColor', 'flat');
    set(gca, 'XTickLabel', implementations);
    title(['Execution Time - ' dataLabels{i}]);
    ylabel('Time (ns)');
    xtickangle(45);
    grid on;
    
    % Use logarithmic scale for larger data sizes
    if i > 3
        set(gca, 'YScale', 'log');
    end
end

% Add a combined plot for larger data sizes
subplot(2, 3, 6);
barh(execTime(:, 4:5), 'FaceColor', 'flat');
set(gca, 'YTickLabel', implementations);
legend(dataLabels(4:5), 'Location', 'eastoutside');
title('Execution Time - Large Data Sizes');
xlabel('Time (ns)');
set(gca, 'XScale', 'log');
grid on;

sgtitle('Execution Time Comparison Across Implementations', 'FontSize', 16);

% Create figure for clock cycles per byte
figure(2);
set(gcf, 'Position', [100, 100, 1200, 800]);

% Plot clock cycles per byte
for i = 1:length(dataSizes)
    subplot(2, 3, i);
    bar(cyclesPerByte(:, i), 'FaceColor', 'flat');
    set(gca, 'XTickLabel', implementations);
    title(['Cycles/Byte - ' dataLabels{i}]);
    ylabel('Cycles/Byte');
    xtickangle(45);
    grid on;
    ylim([0, max(cyclesPerByte(:, i))*1.1]);
end

% Add a combined plot for all data sizes
subplot(2, 3, 6);
plot(1:length(implementations), cyclesPerByte, 'LineWidth', 2, 'Marker', 'o', 'MarkerSize', 8);
set(gca, 'XTick', 1:length(implementations), 'XTickLabel', implementations);
legend(dataLabels, 'Location', 'eastoutside');
title('Cycles/Byte Across Data Sizes');
ylabel('Cycles/Byte');
xtickangle(45);
grid on;

sgtitle('Clock Cycles per Byte Comparison Across Implementations', 'FontSize', 16);

% Create NEW figure for MPI Parallel Efficiency Bar Chart
figure(3);
set(gcf, 'Position', [100, 100, 900, 600]);

bar(parallel_efficiency, 'FaceColor', [0.2 0.6 0.8], 'EdgeColor', 'black', 'LineWidth', 1);
set(gca, 'XTickLabel', mpi_impl_names);
title('MPI Parallel Efficiency Analysis', 'FontSize', 16);
ylabel('Parallel Efficiency (%)', 'FontSize', 12);
xlabel('Implementation', 'FontSize', 12);
xtickangle(45);
grid on;
ylim([0, 110]);

% Add efficiency values on top of bars
for i = 1:length(parallel_efficiency)
    text(i, parallel_efficiency(i) + 2, sprintf('%.1f%%', parallel_efficiency(i)), ...
         'HorizontalAlignment', 'center', 'FontSize', 10, 'FontWeight', 'bold');
end

% Add horizontal line at 100% efficiency
hold on;
plot([0.5, length(parallel_efficiency)+0.5], [100, 100], 'r--', 'LineWidth', 2);
text(length(parallel_efficiency)/2, 102, 'Ideal Efficiency (100%)', ...
     'HorizontalAlignment', 'center', 'Color', 'red', 'FontWeight', 'bold');
hold off;

% Create figure for time speedup
figure(4);
set(gcf, 'Position', [100, 100, 1200, 800]);

% Plot time speedup for each data size
for i = 1:length(dataSizes)
    subplot(2, 3, i);
    bar(speedup_time(:, i), 'FaceColor', 'flat');
    set(gca, 'XTickLabel', implementations);
    title(['Time Speedup vs C Sequential - ' dataLabels{i}]);
    ylabel('Speedup Factor');
    xtickangle(45);
    grid on;
    ylim([0, max(speedup_time(:, i))*1.1]);
end

% Add a combined plot for SELECTED implementations across all data sizes
subplot(2, 3, 6);
% Extract only the selected implementations
selected_speedup_time = speedup_time(selected_impls, :);
plot(log10(dataSizes), selected_speedup_time', 'LineWidth', 2, 'Marker', 'o', 'MarkerSize', 8);
set(gca, 'XTick', log10(dataSizes), 'XTickLabel', dataLabels);
legend(selected_impl_names, 'Location', 'eastoutside');
title('Time Speedup Across Data Sizes (Selected Implementations)');
ylabel('Speedup Factor vs C Sequential');
xtickangle(45);
grid on;

sgtitle('Execution Time Speedup Relative to C Sequential', 'FontSize', 16);

% Create figure for cycles speedup
figure(5);
set(gcf, 'Position', [100, 100, 1200, 800]);

% Plot cycles speedup for each data size
for i = 1:length(dataSizes)
    subplot(2, 3, i);
    bar(speedup_cycles(:, i), 'FaceColor', 'flat');
    set(gca, 'XTickLabel', implementations);
    title(['Cycles/Byte Speedup vs C Sequential - ' dataLabels{i}]);
    ylabel('Speedup Factor');
    xtickangle(45);
    grid on;
    ylim([0, max(speedup_cycles(:, i))*1.1]);
end

% Add a combined plot for SELECTED implementations across all data sizes
subplot(2, 3, 6);
% Extract only the selected implementations
selected_speedup_cycles = speedup_cycles(selected_impls, :);
plot(log10(dataSizes), selected_speedup_cycles', 'LineWidth', 2, 'Marker', 'o', 'MarkerSize', 8);
set(gca, 'XTick', log10(dataSizes), 'XTickLabel', dataLabels);
legend(selected_impl_names, 'Location', 'eastoutside');
title('Cycles/Byte Speedup Across Data Sizes (Selected Implementations)');
ylabel('Speedup Factor vs C Sequential');
xtickangle(45);
grid on;

sgtitle('Clock Cycles per Byte Speedup Relative to C Sequential', 'FontSize', 16);

% Create figure for execution time speedup heatmap
figure(6);
set(gcf, 'Position', [100, 100, 800, 600]);

% Create heatmap for time speedup across implementations and data sizes
imagesc(speedup_time');
colorbar;
title('Execution Time Speedup Heatmap Relative to C Sequential', 'FontSize', 16);
xlabel('Implementation');
ylabel('Data Size');
set(gca, 'XTick', 1:length(implementations), 'XTickLabel', implementations);
set(gca, 'YTick', 1:length(dataLabels), 'YTickLabel', dataLabels);
xtickangle(45);

% Create figure for clock cycles per byte speedup heatmap
figure(7);
set(gcf, 'Position', [100, 100, 800, 600]);

% Create heatmap for cycles speedup across implementations and data sizes
imagesc(speedup_cycles');
colorbar;
title('Clock Cycles per Byte Speedup Heatmap Relative to C Sequential', 'FontSize', 16);
xlabel('Implementation');
ylabel('Data Size');
set(gca, 'XTick', 1:length(implementations), 'XTickLabel', implementations);
set(gca, 'YTick', 1:length(dataLabels), 'YTickLabel', dataLabels);
xtickangle(45);

% Create dedicated figure for time speedup line graph of selected implementations
figure(8);
set(gcf, 'Position', [100, 100, 900, 600]);

% Define color scheme for implementations
colors = lines(length(selected_impls));
markers = {'o', 's', 'd', '^', 'v'};

% Plot time speedup with data size on x-axis and different colors per implementation
hold on;
for i = 1:length(selected_impls)
    plot(dataSizes, selected_speedup_time(i,:), 'Color', colors(i,:), 'LineWidth', 2, ...
         'Marker', markers{i}, 'MarkerSize', 8, 'DisplayName', selected_impl_names{i});
end
hold off;

set(gca, 'XScale', 'log'); % Use log scale for x-axis
grid on;
xlabel('Data Size (bytes)', 'FontSize', 12);
ylabel('Speedup Factor vs C Sequential', 'FontSize', 12);
title('Execution Time Speedup vs Data Size', 'FontSize', 16);
legend('Location', 'best');

% Add data size labels to the x-axis
xticks(dataSizes);
xticklabels({'10', '100', '1,000', '1,000,000', '10,000,000'});
xtickangle(45);

% Create dedicated figure for cycles speedup line graph of selected implementations
figure(9);
set(gcf, 'Position', [100, 100, 900, 600]);

% Plot cycles speedup with data size on x-axis and different colors per implementation
hold on;
for i = 1:length(selected_impls)
    plot(dataSizes, selected_speedup_cycles(i,:), 'Color', colors(i,:), 'LineWidth', 2, ...
         'Marker', markers{i}, 'MarkerSize', 8, 'DisplayName', selected_impl_names{i});
end
hold off;

set(gca, 'XScale', 'log'); % Use log scale for x-axis
grid on;
xlabel('Data Size (bytes)', 'FontSize', 12);
ylabel('Speedup Factor vs C Sequential', 'FontSize', 12);
title('Clock Cycles per Byte Speedup vs Data Size', 'FontSize', 16);
legend('Location', 'best');

% Add data size labels to the x-axis
xticks(dataSizes);
xticklabels({'10', '100', '1,000', '1,000,000', '10,000,000'});
xtickangle(45);

% Save the figures
saveas(figure(1), 'execution_time_comparison.png');
saveas(figure(2), 'cycles_per_byte_comparison.png');
saveas(figure(3), 'mpi_parallel_efficiency.png');
saveas(figure(4), 'time_speedup_comparison.png');
saveas(figure(5), 'cycles_speedup_comparison.png');
saveas(figure(6), 'time_speedup_heatmap.png');
saveas(figure(7), 'cycles_speedup_heatmap.png');
saveas(figure(8), 'time_speedup_vs_datasize.png');
saveas(figure(9), 'cycles_speedup_vs_datasize.png');

% Display parallel efficiency results
fprintf('\nParallel Efficiency Results:\n');
fprintf('============================\n');
for i = 1:length(mpi_impl_names)
    fprintf('%s: %.1f%% efficiency\n', mpi_impl_names{i}, parallel_efficiency(i));
end
