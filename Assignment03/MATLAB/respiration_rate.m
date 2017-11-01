function rr = respiration_rate()

    samples = csvread('..\takashin_Homework_sample.csv',1,0);

    Fs = 50;                    % sample rate in Hz

    RED = samples(:,2);

    Wp = [(10/60)/(Fs/2) (20/60)/(Fs/2)];
    Ws = [(4/60)/(Fs/2) (42/60)/(Fs/2)];
    Rp = 3;
    Rs = 45;

    [n,Wn] = buttord(Wp,Ws,Rp,Rs);
    [b,a] = butter(n,Wn);

    figure
    freqz(b,a)
    % title('Frequency Response of Filter for RR Detection');

    y=filtfilt(b,a,RED);

    figure
    findpeaks(y);
    % title('Filtered RED Signal for RR Detecetion');
    xlabel('sample index');
    ylabel('filtered RED value');

    pks = findpeaks(y);
    rr = length(pks)/1.5;

end