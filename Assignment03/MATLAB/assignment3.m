% Script to run the heart rate, respiration rate, and spo2 functions
% and write the results to CSV

clear

rr = respiration_rate()
hr = heart_rate()
spo2 = spo2()

fid = fopen('..\takashin_Homework_sample.csv');
line1 = fgets(fid);
line2 = fgets(fid);

line_out = line2(3:16);

line_out = [line_out num2str(hr)];
line_out = [line_out line2(17)];

line_out = [line_out num2str(rr)];
line_out = [line_out line2(18)];

line_out = [line_out num2str(spo2)];
line_out = [line_out line2(19:end)];

fclose(fid);

% The provided CSV file mis-labeled IR and RED columns.
% This header has the correct labels.
header = 'RED,IR,HR (bpm), RR (bpm), SPO2 (%)';

fid = fopen("output.csv",'w');
fprintf(fid,'%s\n%s',header,line_out);
fclose(fid);


values = csvread('..\takashin_Homework_sample.csv',1,0);

output=values(2:end,2:3);

dlmwrite('output.csv',output,'-append','precision','%d');