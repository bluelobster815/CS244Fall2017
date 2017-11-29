function hr = heart_rate()

    samples = csvread('walking data.csv',10,0);

    Fs = 50;                    % sample rate in Hz

    RED_raw = samples(:,2);

    Wp = [(60/60)/(Fs/2) (100/60)/(Fs/2)];
    Ws = [(43/60)/(Fs/2) (137/60)/(Fs/2)];
    Rp = 3;
    Rs = 40;

    [n,Wn] = buttord(Wp,Ws,Rp,Rs);
    [b,a] = butter(n,Wn);

    window_size = 12*50;
    slide_distance = 1*50;

    num_full_slides = floor(length(RED_raw)/slide_distance);
    slides_per_window = window_size/slide_distance;

    slides = num_full_slides - slides_per_window + 1;
    
    hr = zeros(1,slides);

    for i=1:slides

        end_index = window_size + slide_distance*(i-1);
        start_index = end_index - window_size + 1;

        RED = RED_raw(start_index:end_index);
    
        y=filtfilt(b,a,RED);

%         figure
%         findpeaks(y);
%         title('Filtered RED Signal for RR Detecetion');
%         xlabel('sample index');
%         ylabel('filtered RED value');
        
        distances = [];
        [pks locs] = findpeaks(y);
        for j=1:length(pks)-1
            distances = [distances locs(j+1)-locs(j)];
        end
        
        hr(i) = Fs * 60 / mean(distances);
%         hr(i) = length(pks)/( length(RED)/Fs/60 );
    
    end

end