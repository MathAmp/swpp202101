define void @f(i32 %x, i32 %y) {
; CHECK-LABEL: @f(i32 %x, i32 %y)
; CHECK-NEXT:    [[F1:%.*]] = add i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[F2:%.*]] = add i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[FCON1:%.*]] = icmp eq i32 [[F1:%.*]], [[X:%.*]]
; CHECK-NEXT:    [[COND:%.*]] = icmp eq i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[FCON2:%.*]] = icmp eq i32 [[F2:%.*]], [[Y:%.*]]
; CHECK-NEXT:    br i1 [[COND]], label [[BB_true_chain1:%.*]], label [[BB_false:%.*]]
; CHECK:       BB_true_chain1:
; CHECK-NEXT:    call void @f(i32 [[X]], i32 [[X]])
; CHECK-NEXT:    br label [[BB_true_chain2:%.*]]
; CHECK:       BB_true_chain2:
; CHECK-NEXT:    call void @f(i32 [[X]], i32 [[X]])
; CHECK-NEXT:    [[M1:%.*]] = add i32 [[X:%.*]], [[X:%.*]]
; CHECK-NEXT:    br i1 true, label [[BB_true_chain3:%.*]], label [[BB_true_chain1:%.*]]
; CHECK:       BB_true_chain3:
; CHECK-NEXT:    [[M2:%.*]] = add i32 [[X:%.*]], [[X:%.*]]
; CHECK-NEXT:    call void @f(i32 [[X]], i32 [[M2]])
; CHECK:       BB_true:
; CHECK-NEXT:    call void @f(i32 [[X]], i32 [[X]])
; CHECK-NEXT:    br label [[BB_end:%.*]]
; CHECK:       BB_false:
; CHECK-NEXT:    call void @f(i32 [[X]], i32 [[Y]])
; CHECK-NEXT:    [[COND2:%.*]] = icmp eq i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    br i1 [[COND2]], label [[BB_false_true:%.*]], label [[BB_false_false:%.*]]
; CHECK:       BB_false_true:
; CHECK-NEXT:    [[TWOX:%.*]] = add i32 [[X:%.*]], [[X:%.*]]
; CHECK-NEXT:    br i1 true, label [[BB_end:%.*]], label [[BB_end_false:%.*]]
; CHECK:       BB_false_false:
; CHECK-NEXT:    [[XY:%.*]] = add i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    br label [[BB_end_false:%.*]]
; CHECK:       BB_end:
; CHECK-NEXT:    call void @f(i32 %x, i32 %y)
; CHECK-NEXT:    ret void
; CHECK:       BB_end_false:
; CHECK-NEXT:    call void @f(i32 %x, i32 %y)
; CHECK-NEXT:    ret void
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
  %cond2 = icmp eq i32 %x, %y
  br i1 %cond2, label %BB_false_true, label %BB_false_false
BB_false_true:
  %two_x = add i32 %x, %y
  br i1 true, label %BB_end, label %BB_end_false
BB_false_false:
  %xy = add i32 %x, %y
  br label %BB_end_false
BB_end:
  call void @f(i32 %x, i32 %y)
  ret void
BB_end_false:
  call void @f(i32 %x, i32 %y)
  ret void
}
