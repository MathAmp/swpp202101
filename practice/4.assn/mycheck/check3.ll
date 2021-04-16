define void @f(i32 %x, i32 %y, i32 %z) {
; CHECK-LABEL: @f(i32 %x, i32 %y, i32 %z)
; CHECK-NEXT:    [[COND1:%.*]] = icmp eq i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    br i1 [[COND1:%.*]], label [[BB_xy_same:%.*]], label [[BB_xy_diff:%.*]]
; CHECK:       BB_xy_same:
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[X:%.*]], i32 [[Z:%.*]])
; CHECK-NEXT:    [[COND2:%.*]] = icmp eq i32 [[X:%.*]], [[Z:%.*]]
; CHECK-NEXT:    br i1 [[COND2:%.*]], label [[BB_xyz_same:%.*]], label [[BB_xy_same_z_diff:%.*]]
; CHECK:       BB_xy_diff:
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[Y:%.*]], i32 [[Z:%.*]])
; CHECK-NEXT:    [[COND3:%.*]] = icmp eq i32 [[Y:%.*]], [[Z:%.*]]
; CHECK-NEXT:    br i1 [[COND3:%.*]], label [[BB_yz_same_x_diff:%.*]], label [[BB_yz_diff_xy_diff:%.*]]
; CHECK:       BB_xyz_same:
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[X:%.*]], i32 [[X:%.*]])
; CHECK-NEXT:    br i1 true, label [[BB_exit:%.*]], label [[BB_xy_same:%.*]]
; CHECK:       BB_xy_same_z_diff:
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[X:%.*]], i32 [[Z:%.*]])
; CHECK-NEXT:    br i1 false, label [[BB_xy_same:%.*]], label [[BB_exit:%.*]]
; CHECK:       BB_yz_same_x_diff
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[Y:%.*]], i32 [[Y:%.*]])
; CHECK-NEXT:    br i1 true, label [[BB_exit:%.*]], label [[BB_exit:%.*]]
; CHECK:       BB_yz_diff_xy_diff
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[Y:%.*]], i32 [[Z:%.*]])
; CHECK-NEXT:    [[COND4:%.*]] = icmp eq i32 [[Z:%.*]], [[X:%.*]]
; CHECK-NEXT:    br i1 [[COND4:%.*]], label [[BB_zx_same_y_diff:%.*]], label [[BB_all_diff:%.*]]
; CHECK:       BB_zx_same_y_diff
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[Y:%.*]], i32 [[X:%.*]])
; CHECK-NEXT:    br i1 false, label [[BB_all_diff:%.*]], label [[BB_exit:%.*]]
; CHECK:       BB_all_diff
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[Y:%.*]], i32 [[Z:%.*]])
; CHECK-NEXT:    br label [[BB_exit:%.*]]
; CHECK:       BB_exit
; CHECK-NEXT:    call void @f(i32 [[X:%.*]], i32 [[Y:%.*]], i32 [[Z:%.*]])
; CHECK-NEXT:    ret void
  %cond1 = icmp eq i32 %x, %y
  br i1 %cond1, label %BB_xy_same, label %BB_xy_diff
BB_xy_same:
  call void @f(i32 %x, i32 %y, i32 %z)
  %cond2 = icmp eq i32 %x, %z
  br i1 %cond2, label %BB_xyz_same, label %BB_xy_same_z_diff
BB_xy_diff:
  call void @f(i32 %x, i32 %y, i32 %z)
  %cond3 = icmp eq i32 %y, %z
  br i1 %cond3, label %BB_yz_same_x_diff, label %BB_yz_diff_xy_diff
BB_xyz_same:
  call void @f(i32 %x, i32 %y, i32 %z)
  br i1 true, label %BB_exit, label %BB_xy_same
BB_xy_same_z_diff:
  call void @f(i32 %x, i32 %y, i32 %z)
  br i1 false, label %BB_xy_same, label %BB_exit
BB_yz_same_x_diff:
  call void @f(i32 %x, i32 %y, i32 %z)
  br i1 true, label %BB_exit, label %BB_exit
BB_yz_diff_xy_diff:
  call void @f(i32 %x, i32 %y, i32 %z)
  %cond4 = icmp eq i32 %z, %x
  br i1 %cond4, label %BB_zx_same_y_diff, label %BB_all_diff
BB_zx_same_y_diff:
  call void @f(i32 %x, i32 %y, i32 %z)
  br i1 false, label %BB_all_diff, label %BB_exit
BB_all_diff:
  call void @f(i32 %x, i32 %y, i32 %z)
  br label %BB_exit
BB_exit:
  call void @f(i32 %x, i32 %y, i32 %z)
  ret void
}