clear all;

%% l2 [Order][Mesh]

%% L1 Rel Error
l2 = ...
    [...
        +1.000000e+00 , +5.554014e-01 , +1.802735e-01 ; ...
        +4.072019e-01 , +5.152727e-02 , +4.676084e-03 ; ...
        +5.152870e-02 , +2.468976e-03 , +1.467633e-04 ; ...
        +2.887924e-03 , +2.358920e-04 , +1.549835e-05 ; ...
        +7.172258e-04 , +4.595670e-05 , +2.837020e-06 ; ...
        +1.905340e-04 , +1.137380e-05 , +7.067146e-07 ; ...
        +5.601543e-05 , +3.495225e-06 , +2.167993e-07 ; ...
        +2.009579e-05 , +1.261267e-06 , +7.771882e-08 ; ...
    ];


%% L1 Abs Error
%l2 = ...
%    [...
%        +3.500348e+04 , +1.944098e+04 , +6.310199e+03 ; ...
%        +1.425348e+04 , +1.803634e+03 , +1.636792e+02 ; ...
%        +1.803684e+03 , +8.642277e+01 , +5.137226e+00 ; ...
%        +1.010874e+02 , +8.257043e+00 , +5.424964e-01 ; ...
%        +2.510540e+01 , +1.608645e+00 , +9.930557e-02 ; ...
%        +6.669354e+00 , +3.981228e-01 , +2.473747e-02 ; ...
%        +1.960735e+00 , +1.223451e-01 , +7.588732e-03 ; ...
%        +7.034226e-01 , +4.414873e-02 , +2.720429e-03 ; ...
%    ];

%% Rel Error
%l2 = ...
%    [...
%        +1.000000e+00 , +5.501554e-01 , +1.743443e-01 , +4.680941e-02 , +1.195660e-02 ; ...
%        +4.291984e-01 , +7.173874e-02 , +1.006197e-02 , +1.397394e-03 , +1.912784e-04 ; ...
%        +9.695191e-02 , +1.006321e-02 , +1.233065e-03 , +1.544785e-04 , +1.935198e-05 ; ...
%        +1.558136e-02 , +2.259740e-03 , +2.921884e-04 , +3.662519e-05 , +4.588253e-06 ; ...
%        +6.131810e-03 , +7.721627e-04 , +9.602295e-05 , +1.200947e-05 , +1.506019e-06 ; ...
%        +2.459011e-03 , +3.079822e-04 , +3.848861e-05 , +4.818164e-06 , +6.061251e-07 ; ...
%        +1.158815e-03 , +1.412935e-04 , +1.767610e-05 , +2.214026e-06 , +2.825523e-07 ; ...
%        +5.629039e-04 , +7.172446e-05 , +8.972703e-06 , +1.125002e-06 , +1.515438e-07 ; ...
%    ];


%% ABS Error
%l2 = ...
%    [...
%        8.23579E+01 , 4.53097E+01 , 1.43586E+01 , 3.85513E+00 , 9.84721E-01 ;...
%        3.53479E+01 , 5.90825E+00 , 8.28683E-01 , 1.15086E-01 , 1.57533E-02 ;...
%        7.98476E+00 , 8.28786E-01 , 1.01553E-01 , 1.27225E-02 , 1.59379E-03 ;...
%        1.28325E+00 , 1.86108E-01 , 2.40640E-02 , 3.01638E-03 , 3.77879E-04 ;...
%        5.05003E-01 , 6.35937E-02 , 7.90825E-03 , 9.89075E-04 , 1.24033E-04 ;...
%        2.02519E-01 , 2.53648E-02 , 3.16984E-03 , 3.96814E-04 , 4.99192E-05 ;...
%        9.54376E-02 , 1.16366E-02 , 1.45577E-03 , 1.82343E-04 , 2.32704E-05 ;...
%        4.63596E-02 , 5.90708E-03 , 7.38973E-04 , 9.26529E-05 , 1.24808E-05 ;...
%    ];

m = [2, 8, 32];%%, 128, 512];
p = [1:8];
O = size(p, 2);
M = size(m, 2);

delta = zeros(O, M - 1);
for i = 1:M-1
    delta(:, i) = ...
        (log10(l2(:, i + 1)) - log10(l2(:, i))) / ...
        (log10(m(i + 1))     - log10(m(i)));
end

delta
loglog(m, l2, '-*')

%loglog(m(1:end - 1), delta, '-*');
%grid