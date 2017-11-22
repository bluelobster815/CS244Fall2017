function rr = respiration_rate()

    samples = csvread('combined2.csv',10,0);

    Fs = 50;                    % sample rate in Hz

    RED_raw = samples(:,2);
    
    window_size = 12 * 50;

    Wp = [(10/60)/(Fs/2) (20/60)/(Fs/2)];
    Ws = [(4/60)/(Fs/2) (42/60)/(Fs/2)];
    Rp = 3;
    Rs = 45;

    [n,Wn] = buttord(Wp,Ws,Rp,Rs);
    [b,a] = butter(n,Wn);

    num_full_windows = floor(length(RED_raw)/window_size);
    
    rr = zeros(1,num_full_windows+1);
    
    for i=1:num_full_windows+1
        
        end_index = i * window_size;
        start_index = end_index - window_size + 1;
        
        if i == num_full_windows+1
            RED = RED_raw(start_index:end);
        else
            RED = RED_raw(start_index:end_index);
        end
    
        y=filtfilt(b,a,RED);

        % figure
        % findpeaks(y);
        % title('Filtered RED Signal for RR Detecetion');
        % xlabel('sample index');
        % ylabel('filtered RED value');
        
        distances = [];
        [pks locs] = findpeaks(y);
        for j=1:length(pks)-1
            distances = [distances locs(j+1)-locs(j)];
        end
        
        rr(i) = Fs * 60 / mean(distances);
%         rr(i) = length(pks)/( length(RED)/Fs/60 );
    
    end    

end