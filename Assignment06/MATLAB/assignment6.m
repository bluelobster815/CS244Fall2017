% Script to run the heart rate, respiration rate, and spo2 functions
% and write the results to CSV

clear

values = csvread('combined2.csv',10,0);

rr = respiration_rate();
hr = heart_rate();
spo2 = spo2();

header = 'IR,RED,X,Y,Z,HR (bpm),RR (bpm),SPO2 (%),error (%)';
fid = fopen("output.csv",'w');
fprintf(fid,'%s\n',header);
fclose(fid);

window_size = 12*50;

num_windows = floor(length(values)/ window_size) + 1;

for i=1:num_windows
    
    line = values(i,:);
    fid = fopen("output.csv",'a');
    fprintf(fid, "%d,%d,%d,%d,%d,%.2f,%.2f,%.2f\n", ...
        line(1),line(2),line(3),line(4),line(5),hr(i),rr(i),spo2(i));
    fclose(fid);

    
end
    
data = values(num_windows+1:end,1:end);

dlmwrite('output.csv',data,'-append','precision','%d');

"done"