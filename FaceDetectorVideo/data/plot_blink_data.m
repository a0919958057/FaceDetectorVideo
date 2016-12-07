clc;clear all;close;

path_blink_data = ['data0.txt';'data1.txt'];
for n = 1:2
    [ms, blink_data]=textread(path_blink_data(n,:),'%f%f','headerlines',0);

    plot(ms ,blink_data);hold on;
    title('\fontsize{14} \fontname{Times New Roman} Blink Matcher');
    xlabel('\fontsize{14} \fontname{Times New Roman} time(ms)');
    ylabel('\fontsize{14} \fontname{Times New Roman} Match value');
    axis([0,ms(end),0,max(blink_data)]);grid on;
end

legend(path_blink_data)