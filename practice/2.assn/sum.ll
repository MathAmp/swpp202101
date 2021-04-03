define double @sum(double* %ptr, i32 %n) {
  %idx = alloca i32
  %res = alloca double
  %c = alloca double
  store i32 0, i32* %idx
  store double 0.0, double* %res
  store double 0.0, double* %c
  br label %comp

comp:
  %1 = load i32, i32* %idx
  %2 = icmp slt i32 %1, %n
  br i1 %2, label %loop, label %exit

loop:
  %3 = load i32, i32* %idx
  %lidx = sext i32 %3 to i64
  %4 = getelementptr inbounds double, double* %ptr, i64 %lidx
  %val = load double, double* %4
  %next = add nsw i32 %3, 1
  store i32 %next, i32* %idx

  %sum = load double, double* %res
  %temp = fadd double %sum, %val
  store double %temp, double* %res

  %asum_1 = fcmp oge double %sum, 0.0
  %asum_2 = fneg double %sum
  %asum = select i1 %asum_1, double %sum, double %asum_2
  %aval_1 = fcmp oge double %val, 0.0
  %aval_2 = fneg double %val
  %aval = select i1 %aval_1, double %val, double %aval_2

  %5 = fcmp oge double %asum, %aval
  br i1 %5, label %cb1, label %cb2

cb1:
  %pc1 = load double, double* %c
  %d1 = fsub double %sum, %temp
  %dt1 = fadd double %d1, %val
  %npc1 = fadd double %pc1, %dt1
  store double %npc1, double* %c
  br label %comp

cb2:
  %pc2 = load double, double* %c
  %d2 = fsub double %val, %temp
  %dt2 = fadd double %d2, %sum
  %npc2 = fadd double %pc2, %dt2
  store double %npc2, double* %c
  br label %comp

exit:
  %6 = load double, double* %res
  %7 = load double, double* %c
  %8 = fadd double %6, %7
  ret double %8
}
