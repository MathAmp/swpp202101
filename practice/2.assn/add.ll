define void @add(i32* %ptr1, i32* %ptr2, i32* %val) {
  %1 = load i32, i32* %val
  %2 = load i32, i32* %ptr1
  %3 = load i32, i32* %ptr2
  %4 = add nsw i32 %1, %2
  %5 = add nsw i32 %1, %3
  store i32 %4, i32* %ptr1
  store i32 %5, i32* %ptr2
  ret void
}
