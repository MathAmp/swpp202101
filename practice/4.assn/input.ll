define void @f(i32 %x, i32 %y) {
  %f1 = add i32 %x, %y
  %f2 = add i32 %x, %y
  %fcon1 = icmp eq i32 %f1, %x
  %cond = icmp eq i32 %x, %y
  %fcon2 = icmp eq i32 %f2, %y
  br i1 %cond, label %BB_true_chain1, label %BB_false
BB_true_chain1:
  call void @f(i32 %x, i32 %y)
  br label %BB_true_chain2
BB_true_chain2:
  call void @f(i32 %x, i32 %y)
  %m1 = add i32 %y, %y
  br i1 true, label %BB_true_chain3, label %BB_true_chain1
BB_true_chain3:
  %m2 = add i32 %y, %y
  call void @f(i32 %y, i32 %m2)
  br i1 false, label %BB_true_chain1, label %BB_true
BB_true:
  call void @f(i32 %x, i32 %y)
  br label %BB_end
BB_false:
  call void @f(i32 %x, i32 %y)
  br label %BB_end
BB_end:
  call void @f(i32 %x, i32 %y)
  ret void
}
