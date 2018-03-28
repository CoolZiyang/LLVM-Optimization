; ModuleID = '/tmp/test1-cdi.bc'
source_filename = "./test1.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@keyArray = internal constant [7 x i32] [i32 29, i32 29, i32 31, i32 31, i32 30, i32 51, i32 2]
@valueArray = internal constant [7 x i32] [i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1]
@keyArray.1 = internal constant [4 x i32] [i32 30, i32 11, i32 31, i32 2]
@valueArray.2 = internal constant [4 x i32] [i32 1, i32 1, i32 1, i32 1]
@keyArray.3 = internal constant [4 x i32] [i32 30, i32 11, i32 31, i32 2]
@valueArray.4 = internal constant [4 x i32] [i32 1, i32 1, i32 1, i32 1]
@keyArray.5 = internal constant [4 x i32] [i32 30, i32 30, i32 11, i32 1]
@valueArray.6 = internal constant [4 x i32] [i32 1, i32 1, i32 1, i32 1]

; Function Attrs: noinline nounwind optnone uwtable
define i32 @_Z3fooj(i32) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store volatile i32 %0, i32* %2, align 4
  store volatile i32 10, i32* %3, align 4
  %4 = load volatile i32, i32* %2, align 4
  %5 = icmp ugt i32 %4, 5
  call void @updateInstrInfo(i32 7, i32* getelementptr inbounds ([7 x i32], [7 x i32]* @keyArray, i32 0, i32 0), i32* getelementptr inbounds ([7 x i32], [7 x i32]* @valueArray, i32 0, i32 0))
  br i1 %5, label %6, label %9

; <label>:6:                                      ; preds = %1
  %7 = load volatile i32, i32* %3, align 4
  %8 = add i32 %7, 5
  store volatile i32 %8, i32* %3, align 4
  call void @updateInstrInfo(i32 4, i32* getelementptr inbounds ([4 x i32], [4 x i32]* @keyArray.1, i32 0, i32 0), i32* getelementptr inbounds ([4 x i32], [4 x i32]* @valueArray.2, i32 0, i32 0))
  br label %12

; <label>:9:                                      ; preds = %1
  %10 = load volatile i32, i32* %3, align 4
  %11 = add i32 %10, 50
  store volatile i32 %11, i32* %3, align 4
  call void @updateInstrInfo(i32 4, i32* getelementptr inbounds ([4 x i32], [4 x i32]* @keyArray.3, i32 0, i32 0), i32* getelementptr inbounds ([4 x i32], [4 x i32]* @valueArray.4, i32 0, i32 0))
  br label %12

; <label>:12:                                     ; preds = %9, %6
  %13 = load volatile i32, i32* %3, align 4
  %14 = load volatile i32, i32* %2, align 4
  %15 = add i32 %13, %14
  call void @updateInstrInfo(i32 4, i32* getelementptr inbounds ([4 x i32], [4 x i32]* @keyArray.5, i32 0, i32 0), i32* getelementptr inbounds ([4 x i32], [4 x i32]* @valueArray.6, i32 0, i32 0))
  call void @printOutInstrInfo()
  ret i32 %15
}

declare void @updateInstrInfo(i32, i32*, i32*)

declare void @printOutInstrInfo()

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.1 (tags/RELEASE_501/final 322011)"}
