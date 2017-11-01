function hr = heart_rate()

    samples = csvread('..\takashin_Homework_sample.csv',1,0);

    Fs = 50;                    % sample rate in Hz

    RED = samples(:,2);

    Wp = [(60/60)/(Fs/2) (100/60)/(Fs/2)];
    Ws = [(43/60)/(Fs/2) (137/60)/(Fs/2)];
    Rp = 3;
    Rs = 40;

    [n,Wn] = buttord(Wp,Ws,Rp,Rs);
    [b,a] = butter(n,Wn);

    figure
    freqz(b,a)
    % title('Frequency Response of Filter for HR Detection');

    y=filtfilt(b,a,RED);

    figure
    findpeaks(y);
    % title('Filtered RED Signal for HR Detecetion');
    xlabel('sample index');
    ylabel('filtered RED value');

    pks = findpeaks(y);
    hr = length(pks)/1.5;

end