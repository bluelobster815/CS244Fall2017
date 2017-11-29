% Script to run the heart rate, respiration rate, and spo2 functions
% and write the results to CSV

clear

values = csvread('walking data.csv',10,0);

rr = respiration_rate();
hr = heart_rate();
spo2 = spo2();

header = 'IR,RED,X,Y,Z,HR (bpm),RR (bpm),SPO2 (%),error (%),mean HR,mean RR,mean SPO2';
fid = fopen("walking output.csv",'w');
fprintf(fid,'%s\n',header);
fclose(fid);

window_size = 12*50;
slide_distance = 1*50;

% num_full_slides = floor(length(spo2)/slide_distance);
% slides_per_window = window_size/slide_distance;
% 
% slides = num_full_slides - slides_per_window + 1;
% 
% num_windows = slides;

slides = min([length(rr) length(hr) length(spo2)]);

xvalues=1:slides;

figure
plot(xvalues,hr(xvalues),xvalues,rr(xvalues),xvalues,spo2(xvalues));
xlabel('slide number');
ylabel('beats/breaths per second, oxygen saturation (%)');
legend('heart rate','respiration rate','spo2','Location','southeast');
ylim([0 inf]);

for i=1:slides
    
    line = values(i,:);
    fid = fopen("walking output.csv",'a');
    if i==1
        fprintf(fid, "%d,%d,%d,%d,%d,%.2f,%.2f,%.2f, ,%.2f,%.2f,%.2f\n", ...
            line(1),line(2),line(3),line(4),line(5),hr(i),rr(i),spo2(i),mean(hr),mean(rr),mean(spo2));
    else
        fprintf(fid, "%d,%d,%d,%d,%d,%.2f,%.2f,%.2f\n", ...
            line(1),line(2),line(3),line(4),line(5),hr(i),rr(i),spo2(i));
    end
    fclose(fid);

    
end
    
data = values(slides+1:end,1:end);

dlmwrite('walking output.csv',data,'-append','precision','%d');

"done"