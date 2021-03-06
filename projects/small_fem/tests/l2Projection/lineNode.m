close all;
clear all;

h = [1, 0.5, 0.25, 0.125, 0.0625];
p = [1:5];

l2 = ...
    [...
        +1.131157e+00 , +9.999165e-01 , +4.203117e-01 , +6.871403e-02 , +1.523634e-02 ; ...
        +1.075659e+00 , +3.597880e-01 , +4.704935e-02 , +8.682276e-03 , +1.250639e-03 ; ...
        +7.064195e-01 , +1.127492e-01 , +1.110520e-02 , +5.421357e-04 , +3.137385e-05 ; ...
        +5.071560e-01 , +3.107026e-02 , +8.747209e-04 , +3.669071e-05 , +1.256389e-06 ; ...
        +1.371007e-01 , +5.591266e-03 , +1.340372e-04 , +1.739642e-06 , +2.567651e-08 ; ...
    ];

P = size(p, 2);
H = size(h, 2);

delta = zeros(P, H - 1);

for i = 1:H-1
  delta(:, i) = ...
    (log10(l2(:, i + 1)) - log10(l2(:, i))) / ...
    (log10(1/h(i + 1))   - log10(1/h(i)));
end

delta

figure;
loglog(1./h, l2', '-*');
grid;
title('line: Nodal');

xlabel('1/h [-]');
ylabel('L2 Error [-]');
