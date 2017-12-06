samples = csvread('sleeping data.csv',10,0);

Fs = 50;

x = samples(:,3)/15987;
y = samples(:,4)/15987;
z = samples(:,5)/15987;

% x = samples(:,1)/15987;
% y = samples(:,2)/15987;
% z = samples(:,3)/15987;

window_size = 12*50;
slide_distance = 1*50;

num_full_slides = floor(length(x)/slide_distance);
slides_per_window = window_size/slide_distance;

slides = num_full_slides - slides_per_window + 1;



mean_x = zeros(slides,1);
mean_y = zeros(slides,1);
mean_z = zeros(slides,1);

var_x = zeros(slides,1);
var_y = zeros(slides,1);
var_z = zeros(slides,1);

mean_pks_a = zeros(slides,1);
mean_val_a = zeros(slides,1);

for i=1:slides
% i=5;
    end_index = window_size + slide_distance*(i-1);
    start_index = end_index - window_size + 1;
    x_window = x(start_index:end_index);
    y_window = y(start_index:end_index);
    z_window = z(start_index:end_index);
    a_window = sqrt((x_window.^2)+(y_window.^2)+(z_window.^2));
    
    mean_x(i) = mean(x_window);
    mean_y(i) = mean(y_window);
    mean_z(i) = mean(z_window);
    
    var_x(i) = var(x_window);
    var_y(i) = var(y_window);
    var_z(i) = var(z_window);
    
    a = a_window;

    Y = fft(a);

    L = length(a);
    P2 = abs(Y/L);
    P1 = P2(1:L/2+1);
    P1(2:end-1) = 2*P1(2:end-1);

    f = Fs*(0:(L/2))/L;
%     plot(f,P1)
    [fftpeak index] = max(P1(5:end));
    peak_freq = f(index+4);
    min_peak_distance = (1/peak_freq)*0.6*Fs;
    
    [pks pk_locs] = findpeaks(a_window,'MinPeakDistance',min_peak_distance);
    [vals val_locs] = findpeaks(-a_window,'MinPeakDistance',min_peak_distance);
    vals = -vals;
    
    mean_pks_a(i) = mean(pks);
    mean_val_a(i) = mean(vals);
    
end

feature = horzcat(mean_x,mean_y,mean_z,var_x,var_y,var_z,mean_pks_a,mean_val_a);

dlmwrite('sleeping_features.csv',feature,'delimiter',',','precision',5);

clear;

%*******************************************************************************

samples = csvread('sitting data.csv',10,0);

Fs = 50;

x = samples(:,3)/15987;
y = samples(:,4)/15987;
z = samples(:,5)/15987;

% x = samples(:,1)/15987;
% y = samples(:,2)/15987;
% z = samples(:,3)/15987;

window_size = 12*50;
slide_distance = 1*50;

num_full_slides = floor(length(x)/slide_distance);
slides_per_window = window_size/slide_distance;

slides = num_full_slides - slides_per_window + 1;



mean_x = zeros(slides,1);
mean_y = zeros(slides,1);
mean_z = zeros(slides,1);

var_x = zeros(slides,1);
var_y = zeros(slides,1);
var_z = zeros(slides,1);

mean_pks_a = zeros(slides,1);
mean_val_a = zeros(slides,1);

for i=1:slides
% i=5;
    end_index = window_size + slide_distance*(i-1);
    start_index = end_index - window_size + 1;
    x_window = x(start_index:end_index);
    y_window = y(start_index:end_index);
    z_window = z(start_index:end_index);
    a_window = sqrt((x_window.^2)+(y_window.^2)+(z_window.^2));
    
    mean_x(i) = mean(x_window);
    mean_y(i) = mean(y_window);
    mean_z(i) = mean(z_window);
    
    var_x(i) = var(x_window);
    var_y(i) = var(y_window);
    var_z(i) = var(z_window);
    
    a = a_window;

    Y = fft(a);

    L = length(a);
    P2 = abs(Y/L);
    P1 = P2(1:L/2+1);
    P1(2:end-1) = 2*P1(2:end-1);

    f = Fs*(0:(L/2))/L;
%     plot(f,P1)
    [fftpeak index] = max(P1(5:end));
    peak_freq = f(index+4);
    min_peak_distance = (1/peak_freq)*0.6*Fs;
    
    [pks pk_locs] = findpeaks(a_window,'MinPeakDistance',min_peak_distance);
    [vals val_locs] = findpeaks(-a_window,'MinPeakDistance',min_peak_distance);
    vals = -vals;
    
    mean_pks_a(i) = mean(pks);
    mean_val_a(i) = mean(vals);
    
end

feature = horzcat(mean_x,mean_y,mean_z,var_x,var_y,var_z,mean_pks_a,mean_val_a);

dlmwrite('sitting_features.csv',feature,'delimiter',',','precision',5);

clear;

%*******************************************************************************

samples = csvread('standing data.csv',10,0);

Fs = 50;

x = samples(:,3)/15987;
y = samples(:,4)/15987;
z = samples(:,5)/15987;

% x = samples(:,1)/15987;
% y = samples(:,2)/15987;
% z = samples(:,3)/15987;

window_size = 12*50;
slide_distance = 1*50;

num_full_slides = floor(length(x)/slide_distance);
slides_per_window = window_size/slide_distance;

slides = num_full_slides - slides_per_window + 1;



mean_x = zeros(slides,1);
mean_y = zeros(slides,1);
mean_z = zeros(slides,1);

var_x = zeros(slides,1);
var_y = zeros(slides,1);
var_z = zeros(slides,1);

mean_pks_a = zeros(slides,1);
mean_val_a = zeros(slides,1);

for i=1:slides
% i=5;
    end_index = window_size + slide_distance*(i-1);
    start_index = end_index - window_size + 1;
    x_window = x(start_index:end_index);
    y_window = y(start_index:end_index);
    z_window = z(start_index:end_index);
    a_window = sqrt((x_window.^2)+(y_window.^2)+(z_window.^2));
    
    mean_x(i) = mean(x_window);
    mean_y(i) = mean(y_window);
    mean_z(i) = mean(z_window);
    
    var_x(i) = var(x_window);
    var_y(i) = var(y_window);
    var_z(i) = var(z_window);
    
    a = a_window;

    Y = fft(a);

    L = length(a);
    P2 = abs(Y/L);
    P1 = P2(1:L/2+1);
    P1(2:end-1) = 2*P1(2:end-1);

    f = Fs*(0:(L/2))/L;
%     plot(f,P1)
    [fftpeak index] = max(P1(5:end));
    peak_freq = f(index+4);
    min_peak_distance = (1/peak_freq)*0.6*Fs;
    
    [pks pk_locs] = findpeaks(a_window,'MinPeakDistance',min_peak_distance);
    [vals val_locs] = findpeaks(-a_window,'MinPeakDistance',min_peak_distance);
    vals = -vals;
    
    mean_pks_a(i) = mean(pks);
    mean_val_a(i) = mean(vals);
    
end

feature = horzcat(mean_x,mean_y,mean_z,var_x,var_y,var_z,mean_pks_a,mean_val_a);

dlmwrite('standing_features.csv',feature,'delimiter',',','precision',5);

clear;

%*******************************************************************************

samples = csvread('walking data.csv',10,0);

Fs = 50;

x = samples(:,3)/15987;
y = samples(:,4)/15987;
z = samples(:,5)/15987;

% x = samples(:,1)/15987;
% y = samples(:,2)/15987;
% z = samples(:,3)/15987;

window_size = 12*50;
slide_distance = 1*50;

num_full_slides = floor(length(x)/slide_distance);
slides_per_window = window_size/slide_distance;

slides = num_full_slides - slides_per_window + 1;



mean_x = zeros(slides,1);
mean_y = zeros(slides,1);
mean_z = zeros(slides,1);

var_x = zeros(slides,1);
var_y = zeros(slides,1);
var_z = zeros(slides,1);

mean_pks_a = zeros(slides,1);
mean_val_a = zeros(slides,1);

for i=1:slides
% i=5;
    end_index = window_size + slide_distance*(i-1);
    start_index = end_index - window_size + 1;
    x_window = x(start_index:end_index);
    y_window = y(start_index:end_index);
    z_window = z(start_index:end_index);
    a_window = sqrt((x_window.^2)+(y_window.^2)+(z_window.^2));
    
    mean_x(i) = mean(x_window);
    mean_y(i) = mean(y_window);
    mean_z(i) = mean(z_window);
    
    var_x(i) = var(x_window);
    var_y(i) = var(y_window);
    var_z(i) = var(z_window);
    
    a = a_window;

    Y = fft(a);

    L = length(a);
    P2 = abs(Y/L);
    P1 = P2(1:L/2+1);
    P1(2:end-1) = 2*P1(2:end-1);

    f = Fs*(0:(L/2))/L;
%     plot(f,P1)
    [fftpeak index] = max(P1(5:end));
    peak_freq = f(index+4);
    min_peak_distance = (1/peak_freq)*0.6*Fs;
    
    [pks pk_locs] = findpeaks(a_window,'MinPeakDistance',min_peak_distance);
    [vals val_locs] = findpeaks(-a_window,'MinPeakDistance',min_peak_distance);
    vals = -vals;
    
    mean_pks_a(i) = mean(pks);
    mean_val_a(i) = mean(vals);
    
end

feature = horzcat(mean_x,mean_y,mean_z,var_x,var_y,var_z,mean_pks_a,mean_val_a);

dlmwrite('walking_features.csv',feature,'delimiter',',','precision',5);

clear;

%*******************************************************************************