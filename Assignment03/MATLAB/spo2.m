function spo2 = spo2()
    
    samples = csvread('..\takashin_Homework_sample.csv',1,0);
    
    Fs = 50;

    IR = samples(:,3);
    RED = samples(:,2);

    IR_avg = [];
    RED_avg = [];

    % moving average to smooth out the signal without using frequency filters
    for i = 1:length(IR)-4
       IR_avg(i) = ( IR(i) + IR(i+1) + IR(i+2) + IR(i+3) ) / 4;
       RED_avg(i) = ( RED(i) + RED(i+1) + RED(i+2) + RED(i+3) ) / 4;
    end
    
    figure
    plot(IR_avg);
    % title('Smoothed IR Signal for SPO2 Calculation');
    xlabel('sample index');
    ylabel('raw IR value');

    % find peaks and peak locations in averaged IR signal
    [IR_pks IR_pk_locs] = findpeaks(IR_avg,'MinPeakDistance',35);
    % invert signal and apply peak detection to find valleys
    [IR_vals IR_val_locs] = findpeaks(-IR_avg,'MinPeakDistance',35);
    % invert inverted valley values to restore them to normal
    IR_vals = -IR_vals;

    % do the same as above to RED signal
    [RED_pks RED_pk_locs] = findpeaks(RED_avg,'MinPeakDistance',35);
    [RED_vals RED_val_locs] = findpeaks(-RED_avg,'MinPeakDistance',35);
    RED_vals = -RED_vals;

    % drop the first peak location since it cannot be used to calculate SPO2
    IR_pk_locs = IR_pk_locs(2:end);
    IR_pks = IR_pks(2:end);

    RED_pk_locs = RED_pk_locs(2:end);
    RED_pks = RED_pks(2:end);

    R = [];

    for i=1:min(length(IR_pk_locs),length(RED_pk_locs))

        % check to make sure that each peak being used is between two valleys
        assert(IR_val_locs(i)<IR_pk_locs(i))
        assert(IR_val_locs(i+1)>IR_pk_locs(i))

        assert(RED_val_locs(i)<RED_pk_locs(i))
        assert(RED_val_locs(i+1)>RED_pk_locs(i))

        % use slope-intercept form of line equation to solve for DC component
        % y = mx + b
        % b = y - mx

        slope_IR = ( IR_vals(i+1) - IR_vals(i) ) / ( IR_val_locs(i+1) - IR_val_locs(i) );
        % DC_IR = slope_IR * ( IR_pk_locs(i) - IR_val_locs(i) ) + IR_vals(i);
        b_IR = IR_vals(i) - slope_IR * IR_val_locs(i);
        DC_IR = slope_IR * IR_pk_locs(i) + b_IR;    
        AC_IR = IR_pks(i) - DC_IR;

        slope_RED = ( RED_vals(i+1) - RED_vals(i) ) / ( RED_val_locs(i+1) - RED_val_locs(i) );
        % DC_RED = slope_RED * ( RED_pk_locs(i) - RED_val_locs(i) ) + RED_vals(i);    
        b_RED = RED_vals(i) - slope_RED * RED_val_locs(i);
        DC_RED = slope_RED * RED_pk_locs(i) + b_RED;    
        AC_RED = RED_pks(i) - DC_RED;

        num = ( AC_RED * DC_IR );
        den = ( AC_IR * DC_RED );
        ratio = num/den;
        R = [R ratio];

    end
    
    Rmean = mean(R);
    
    spo2 = ( -45.060 * Rmean * Rmean + 30.354 * Rmean + 94.845 );

%     spo2_values = [];
% 
%     for i=1:length(R)
%         spo2_values = [spo2_values ( -45.060 * R(i) * R(i) + 30.354 * R(i) + 94.845 )]
%     end
    
end