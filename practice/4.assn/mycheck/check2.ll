define void @f(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: @f(i32 %a, i32 %b, i32 %c, i32 %d)
; CHECK-NEXT:    [[COND_AB:%.*]] = icmp eq i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[T1:%.*]] = add i32 [[C:%.*]], [[D:%.*]]
; CHECK-NEXT:    [[COND_CD:%.*]] = icmp eq i32 [[C:%.*]], [[T1:%.*]]
; CHECK-NEXT:    [[COND1:%.*]] = and i1 [[COND_AB:%.*]], [[COND_CD:%.*]]
; CHECK-NEXT:    br i1 [[COND1]], label [[BB_true:%.*]], label [[BB_false:%.*]]
; CHECK:       BB_true:
; CHECK-NEXT:    [[X:%.*]] = add i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[Y:%.*]] = add i32 [[T1:%.*]], [[D:%.*]]
; CHECK-NEXT:    [[COND2:%.*]] = icmp eq i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    br i1 [[COND2:%.*]], label [[BB_true_true:%.*]], label [[BB_true_false:%.*]]
; CHECK:       BB_true_true:
; CHECK-NEXT:    [[Z:%.*]] = add i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    br label [[BB_false:%.*]]
; CHECK:       BB_true_false:
; CHECK-NEXT:    [[COND4:%.*]] = icmp eq i32 [[A:%.*]], [[C:%.*]]
; CHECK-NEXT:    br i1 [[COND4:%.*]], label [[BB_true_true:%.*]], label [[BB_false:%.*]]
; CHECK:       BB_false:
; CHECK-NEXT:    [[COND_AA:%.*]] = icmp eq i32 [[A:%.*]], [[A:%.*]]
; CHECK-NEXT:    br i1 [[COND_AA:%.*]], label [[BB_false_true:%.*]], label [[BB_false_false:%.*]]
; CHECK:       BB_false_true:
; CHECK-NEXT:    [[R1:%.*]] = add i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    ret void
; CHECK:       BB_false_false:
; CHECK-NEXT:    [[R2:%.*]] = add i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[COND_BD:%.*]] = icmp eq i32 [[B:%.*]], [[R2:%.*]]
; CHECK-NEXT:    br i1 [[COND_BD:%.*]], label [[Ret1:%.*]], label [[Ret2:%.*]]
; CHECK:       Ret1:
; CHECK-NEXT:    [[R3:%.*]] = add i32 [[B:%.*]], [[B:%.*]]
; CHECK_NEXT:    ret void
; CHECK:       Ret2:
; CHECK-NEXT:    [[R3:%.*]] = add i32 [[R2:%.*]], [[R2:%.*]]
; CHECK_NEXT:    ret void
  %cond_ab = icmp eq i32 %a, %b
  %t1 = add i32 %c, %d
  %cond_cd = icmp eq i32 %c, %t1
  %cond1 = and i1 %cond_ab, %cond_cd
  br i1 %cond1, label %BB_true, label %BB_false
BB_true:
  %x = add i32 %a, %b
  %y = add i32 %t1, %d
  %cond2 = icmp eq i32 %x, %y
  br i1 %cond2, label %BB_true_true, label %BB_true_false
BB_true_true:
  %z = add i32 %x, %y
  br label %BB_false
BB_true_false:
  %cond4 = icmp eq i32 %a, %c
  br i1 %cond4, label %BB_true_true, label %BB_false
BB_false:
  %cond_aa = icmp eq i32 %a, %a
  br i1 %cond_aa, label %BB_false_true, label %BB_false_false
BB_false_true:
  %r1 = add i32 %a, %b
  ret void
BB_false_false:
  %r2 = add i32 %a, %b
  %cond_bd = icmp eq i32 %b, %r2
  br i1 %cond_bd, label %Ret1, label %Ret2
Ret1:
  %r3 = add i32 %r2, %r2
  ret void
Ret2:
  %r4 = add i32 %r2, %r2
  ret void
}
