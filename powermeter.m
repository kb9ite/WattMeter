refActADC = [8 9 10 15 20 27 42 60 85 135 200 310 435 545 630 720 790 930];
refActVpp = [3.8 5.2 6 8.5 9.7 11.4 15.2 20 25 36 49 74.5 100 120 140 160 175 200];

fwdActADC = [9 10 14 23 30 38 46 62 82 104 147 195 290 340 430 520 611 710 750 905];
fwdActVpp = [4.2 5.65 8 10 12 14 16 20 25 30 40 50 70 80 100 120 140 160 170 200];

diary on
fprintf('Reflected cal\n')
CurveFit(refActADC, refActVpp)

fprintf('Forward cal\n');
CurveFit(fwdActADC, fwdActVpp)
diary off

function CurveFit(actADC, actVpp)
splineSwPt = 300;


pts = actADC < splineSwPt;
pp = spline(actADC, actVpp);

p = polyfit(actADC(~pts), actVpp(~pts), 1);

adc = 0:4095;
v = [ppval(pp, 0:(splineSwPt-1)) p(1).*(splineSwPt:4095) + p(2)];

plot(actADC, (actVpp/2/sqrt(2)).^2/50, 'x', adc, (v/2/sqrt(2)).^2/50)


pwr = (v/2/sqrt(2)).^2 / 50 * 2^16;
for p = pwr
    fprintf('%d,\n', round(p,0));
end
end