function spo2 = spo2_sleeping()
    
    samples = csvread('sleeping data.csv',10,0);
    
    Fs = 50;

    IR = samples(:,1);
    RED = samples(:,2);

    IR_avg = zeros(1,length(IR)-4);
    RED_avg = zeros(1,length(IR)-4);

    % moving average to smooth out the signal without using frequency filters
    for i = 1:length(IR)-4
       IR_avg(i) = ( IR(i) + IR(i+1) + IR(i+2) + IR(i+3) ) / 4;
       RED_avg(i) = ( RED(i) + RED(i+1) + RED(i+2) + RED(i+3) ) / 4;
    end

    window_size = 12*50;
    slide_distance = 1*50;

    num_full_slides = floor(length(RED_avg)/slide_distance);
    slides_per_window = window_size/slide_distance;

    slides = num_full_slides - slides_per_window + 1;
    
    spo2s = zeros(1,slides);
    
    Rmeans = [];
    
    for j=1:slides

        end_index = window_size + slide_distance*(j-1);
        start_index = end_index - window_size + 1;

        IR_window = IR_avg(start_index:end_index);
        RED_window = RED_avg(start_index:end_index);

        % invert signal and apply peak detection to find valleys
        [IR_vals IR_val_locs] = findpeaks(-IR_window,'MinPeakDistance',20);
        % invert inverted valley values to restore them to normal
        IR_vals = -IR_vals;

        % do the same as above to RED signal
        [RED_vals RED_val_locs] = findpeaks(-RED_window,'MinPeakDistance',20);
        RED_vals = -RED_vals;

        n = min(length(IR_val_locs)-1,length(RED_val_locs)-1);
        
        R = [];

        for i=1:n

            % use slope-intercept form of line equation to solve for DC component
            % y = mx + b
            % b = y - mx

            % find the slope of the line between two valleys
            slope_IR = ( IR_vals(i+1) - IR_vals(i) ) / ( IR_val_locs(i+1) - IR_val_locs(i) );
            % search for the peak between the two valleys and get the index
            [IR_peak pk_index] = max(IR_window(IR_val_locs(i):IR_val_locs(i+1)));
            IR_peak_loc = IR_val_locs(i) + pk_index - 1;
            % solve the line equation
            b_IR = IR_vals(i) - slope_IR * IR_val_locs(i);
            DC_IR = slope_IR * IR_peak_loc + b_IR;    
            AC_IR = IR_peak - DC_IR;

            % do the same for RED
            slope_RED = ( RED_vals(i+1) - RED_vals(i) ) / ( RED_val_locs(i+1) - RED_val_locs(i) );
            [RED_peak pk_index] = max(RED_window(RED_val_locs(i):RED_val_locs(i+1)));
            RED_peak_loc = RED_val_locs(i) + pk_index - 1;
            b_RED = RED_vals(i) - slope_RED * RED_val_locs(i);
            DC_RED = slope_RED * RED_peak_loc + b_RED;    
            AC_RED = RED_peak - DC_RED;

            num = ( AC_RED * DC_IR );
            den = ( AC_IR * DC_RED );
            ratio = num/den;
            R = [R ratio];

        end

        Rmean = mean(R);
        
        Rmeans = [Rmeans Rmean];

        spo2s(j) = ( -45.060 * Rmean * Rmean + 30.354 * Rmean + 94.845 );
        
    end
    
    spo2=spo2s;
    
end