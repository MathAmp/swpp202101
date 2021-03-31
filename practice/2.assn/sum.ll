define double @sum(double* %ptr, i32 %n) {
  %idx = alloca i32
  %res = alloca double
  store i32 0, i32* %idx
  store double 0.0, double* %res
  br label %comp

comp:
  %1 = load i32, i32* %idx
  %2 = icmp slt i32 %1, %n
  br i1 %2, label %loop, label %exit

loop:
  %3 = load i32, i32* %idx
  %lidx = sext i32 %3 to i64
  %4 = getelementptr inbounds double, double* %ptr, i64 %lidx
  %5 = load double, double* %res
  %6 = load double, double* %4
  %s = fadd double %5, %6
  store double %s, double* %res
  %next = add nsw i32 %3, 1
  store i32 %next, i32* %idx
  br label %comp
  

exit:
  %7 = load double, double* %res
  ret double %7
}
