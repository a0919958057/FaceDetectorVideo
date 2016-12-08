clc;clear all;close;

path_blink_data = ['data0.txt';'data1.txt'];
for n = 1:2
    [ms, blink_data]=textread(path_blink_data(n,:),'%f%f','headerlines',0);

    windowSize = 30;
    b = (1/windowSize)*ones(1,windowSize);
    a = 1;
    blink_data_filter = filter(b,a,blink_data);  
    
    subplot(2,1,n);
    plot(ms , blink_data);hold on;
    title('\fontsize{14} \fontname{Times New Roman} Blink Matcher');
    xlabel('\fontsize{14} \fontname{Times New Roman} time(ms)');
    ylabel('\fontsize{14} \fontname{Times New Roman} Match value');
    axis([0,ms(end),min(blink_data),max(blink_data)]);grid on;
    legend(path_blink_data(n,:))
end

