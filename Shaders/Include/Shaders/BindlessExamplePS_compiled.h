#if 0
;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; COLOUR                   0   xyzw        0     NONE   float   xyzw
; TEXCOORD                 0   xy          1     NONE   float   xy  
;
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; SV_Target                0   xyzw        0   TARGET   float   xyzw
;
; shader hash: 38cf8fd6872405327fae624c98c969fd
;
; Pipeline Runtime Information: 
;
; Pixel Shader
; DepthOutput=0
; SampleFrequency=0
;
;
; Input signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; COLOUR                   0                 linear       
; TEXCOORD                 0                 linear       
;
; Output signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; SV_Target                0                              
;
; Buffer Definitions:
;
; cbuffer MaterialInfoCB
; {
;
;   struct MaterialInfoCB
;   {
;
;       struct struct.MaterialInfo
;       {
;
;           uint albedoTextureIndex;                  ; Offset:    0
;           float3 _emtpty;                           ; Offset:    4
;       
;       } MaterialInfoCB;                             ; Offset:    0
;
;   
;   } MaterialInfoCB;                                 ; Offset:    0 Size:    16
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; MaterialInfoCB                    cbuffer      NA          NA     CB0            cb0     1
; DefaultSampler                    sampler      NA          NA      S0             s0     1
; Texture2DTable                    texture     f32          2d      T0    t0,space100unbounded
;
;
; ViewId state:
;
; Number of inputs: 6, outputs: 4
; Outputs dependent on ViewId: {  }
; Inputs contributing to computation of Outputs:
;   output 0 depends on inputs: { 0, 4, 5 }
;   output 1 depends on inputs: { 1, 4, 5 }
;   output 2 depends on inputs: { 2, 4, 5 }
;   output 3 depends on inputs: { 3, 4, 5 }
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%MaterialInfoCB = type { %struct.MaterialInfo }
%struct.MaterialInfo = type { i32, <3 x float> }
%struct.SamplerState = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 3, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %3 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %5 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %7 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %9 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %2, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  %10 = extractvalue %dx.types.CBufRet.i32 %9, 0
  %11 = icmp eq i32 %10, -1
  br i1 %11, label %20, label %12

; <label>:12                                      ; preds = %0
  %13 = add i32 %10, 0
  %14 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 %13, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %15 = call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, %dx.types.Handle %14, %dx.types.Handle %1, float %3, float %4, float undef, float undef, i32 0, i32 0, i32 undef, float undef)  ; Sample(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,clamp)
  %16 = extractvalue %dx.types.ResRet.f32 %15, 0
  %17 = extractvalue %dx.types.ResRet.f32 %15, 1
  %18 = extractvalue %dx.types.ResRet.f32 %15, 2
  %19 = extractvalue %dx.types.ResRet.f32 %15, 3
  br label %20

; <label>:20                                      ; preds = %12, %0
  %21 = phi float [ %16, %12 ], [ %5, %0 ]
  %22 = phi float [ %17, %12 ], [ %6, %0 ]
  %23 = phi float [ %18, %12 ], [ %7, %0 ]
  %24 = phi float [ %19, %12 ], [ %8, %0 ]
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %21)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %22)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %23)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %24)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sample.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #2

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.viewIdState = !{!11}
!dx.entryPoints = !{!12}

!0 = !{!"dxc 1.2"}
!1 = !{i32 1, i32 5}
!2 = !{!"ps", i32 6, i32 5}
!3 = !{!4, null, !7, !9}
!4 = !{!5}
!5 = !{i32 0, [0 x %"class.Texture2D<vector<float, 4> >"]* undef, !"", i32 100, i32 0, i32 -1, i32 2, i32 0, !6}
!6 = !{i32 0, i32 9}
!7 = !{!8}
!8 = !{i32 0, %MaterialInfoCB* undef, !"", i32 0, i32 0, i32 1, i32 16, null}
!9 = !{!10}
!10 = !{i32 0, %struct.SamplerState* undef, !"", i32 0, i32 0, i32 1, i32 0, null}
!11 = !{[8 x i32] [i32 6, i32 4, i32 1, i32 2, i32 4, i32 8, i32 15, i32 15]}
!12 = !{void ()* @main, !"main", !13, !3, null}
!13 = !{!14, !20, null}
!14 = !{!15, !18}
!15 = !{i32 0, !"COLOUR", i8 9, i8 0, !16, i8 2, i32 1, i8 4, i32 0, i8 0, !17}
!16 = !{i32 0}
!17 = !{i32 3, i32 15}
!18 = !{i32 1, !"TEXCOORD", i8 9, i8 0, !16, i8 2, i32 1, i8 2, i32 1, i8 0, !19}
!19 = !{i32 3, i32 3}
!20 = !{!21}
!21 = !{i32 0, !"SV_Target", i8 9, i8 16, !16, i8 0, i32 1, i8 4, i32 0, i8 0, !17}

#endif

const unsigned char gBindlessExamplePS[] = {
  0x44, 0x58, 0x42, 0x43, 0xdb, 0xd7, 0x13, 0x86, 0xdc, 0x48, 0x60, 0x59,
  0x21, 0x1b, 0x74, 0xe5, 0xd7, 0xf6, 0x24, 0xb7, 0x01, 0x00, 0x00, 0x00,
  0xa6, 0x11, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x4c, 0x00, 0x00, 0x00, 0xac, 0x00, 0x00, 0x00, 0xe6, 0x00, 0x00, 0x00,
  0xc2, 0x01, 0x00, 0x00, 0xfa, 0x09, 0x00, 0x00, 0x16, 0x0a, 0x00, 0x00,
  0x53, 0x46, 0x49, 0x30, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x49, 0x53, 0x47, 0x31, 0x58, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x43, 0x4f, 0x4c, 0x4f, 0x55, 0x52, 0x00, 0x54, 0x45, 0x58, 0x43, 0x4f,
  0x4f, 0x52, 0x44, 0x00, 0x4f, 0x53, 0x47, 0x31, 0x32, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x53, 0x56, 0x5f, 0x54, 0x61, 0x72, 0x67, 0x65,
  0x74, 0x00, 0x50, 0x53, 0x56, 0x30, 0xd4, 0x00, 0x00, 0x00, 0x24, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x02, 0x01, 0x00,
  0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x14, 0x00,
  0x00, 0x00, 0x00, 0x43, 0x4f, 0x4c, 0x4f, 0x55, 0x52, 0x00, 0x54, 0x45,
  0x58, 0x43, 0x4f, 0x4f, 0x52, 0x44, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x44, 0x00, 0x03, 0x02,
  0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x42, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x44, 0x10, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x54, 0x41, 0x54, 0x30, 0x08,
  0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x0c, 0x02, 0x00, 0x00, 0x44, 0x58,
  0x49, 0x4c, 0x05, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x18, 0x08,
  0x00, 0x00, 0x42, 0x43, 0xc0, 0xde, 0x21, 0x0c, 0x00, 0x00, 0x03, 0x02,
  0x00, 0x00, 0x0b, 0x82, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13, 0x00,
  0x00, 0x00, 0x07, 0x81, 0x23, 0x91, 0x41, 0xc8, 0x04, 0x49, 0x06, 0x10,
  0x32, 0x39, 0x92, 0x01, 0x84, 0x0c, 0x25, 0x05, 0x08, 0x19, 0x1e, 0x04,
  0x8b, 0x62, 0x80, 0x18, 0x45, 0x02, 0x42, 0x92, 0x0b, 0x42, 0xc4, 0x10,
  0x32, 0x14, 0x38, 0x08, 0x18, 0x4b, 0x0a, 0x32, 0x62, 0x88, 0x48, 0x90,
  0x14, 0x20, 0x43, 0x46, 0x88, 0xa5, 0x00, 0x19, 0x32, 0x42, 0xe4, 0x48,
  0x0e, 0x90, 0x11, 0x23, 0xc4, 0x50, 0x41, 0x51, 0x81, 0x8c, 0xe1, 0x83,
  0xe5, 0x8a, 0x04, 0x31, 0x46, 0x06, 0x51, 0x18, 0x00, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x1b, 0x8c, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x40, 0x02,
  0xa8, 0x0d, 0x84, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x03, 0x20, 0x6d, 0x30,
  0x86, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x09, 0xa8, 0x00, 0x49, 0x18,
  0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x13, 0x82, 0x60, 0x42, 0x20, 0x4c,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x89, 0x20, 0x00, 0x00, 0x5a, 0x00,
  0x00, 0x00, 0x32, 0x22, 0x88, 0x09, 0x20, 0x64, 0x85, 0x04, 0x13, 0x23,
  0xa4, 0x84, 0x04, 0x13, 0x23, 0xe3, 0x84, 0xa1, 0x90, 0x14, 0x12, 0x4c,
  0x8c, 0x8c, 0x0b, 0x84, 0xc4, 0x4c, 0x10, 0x90, 0xc1, 0x0c, 0xc0, 0x30,
  0x02, 0x01, 0xcc, 0x11, 0x80, 0xc1, 0x4c, 0x6d, 0x30, 0x0e, 0xec, 0x10,
  0x0e, 0xf3, 0x30, 0x0f, 0x6e, 0x40, 0x0b, 0xe5, 0x80, 0x0f, 0xf4, 0x50,
  0x0f, 0xf2, 0x50, 0x0e, 0x72, 0x40, 0x0a, 0x7c, 0x60, 0x0f, 0xe5, 0x30,
  0x0e, 0xf4, 0xf0, 0x0e, 0xf2, 0xc0, 0x07, 0xe6, 0xc0, 0x0e, 0xef, 0x10,
  0x0e, 0xf4, 0xc0, 0x06, 0x60, 0x40, 0x07, 0x7e, 0x00, 0x06, 0x7e, 0xa0,
  0x07, 0x7a, 0xd0, 0x0e, 0xe9, 0x00, 0x0f, 0xf3, 0xf0, 0x0b, 0xf4, 0x90,
  0x0f, 0xf0, 0x50, 0x0e, 0x28, 0x10, 0x66, 0x12, 0x83, 0x71, 0x60, 0x87,
  0x70, 0x98, 0x87, 0x79, 0x70, 0x03, 0x5a, 0x28, 0x07, 0x7c, 0xa0, 0x87,
  0x7a, 0x90, 0x87, 0x72, 0x90, 0x03, 0x52, 0xe0, 0x03, 0x7b, 0x28, 0x87,
  0x71, 0xa0, 0x87, 0x77, 0x90, 0x07, 0x3e, 0x30, 0x07, 0x76, 0x78, 0x87,
  0x70, 0xa0, 0x07, 0x36, 0x00, 0x03, 0x3a, 0xf0, 0x03, 0x30, 0xf0, 0x03,
  0x24, 0x04, 0x43, 0x02, 0x10, 0x54, 0xdc, 0x24, 0x4d, 0x11, 0x25, 0x4c,
  0x3e, 0x0b, 0x30, 0xcf, 0x42, 0x44, 0xec, 0x04, 0x4c, 0x04, 0x0a, 0x04,
  0x3a, 0x86, 0x11, 0x06, 0xe0, 0x26, 0x69, 0x8a, 0x28, 0x61, 0xf2, 0x4d,
  0x60, 0x22, 0x22, 0x04, 0x58, 0xc4, 0xa6, 0x70, 0x90, 0x20, 0x24, 0xe7,
  0x98, 0xc0, 0x44, 0x44, 0x08, 0xb0, 0x88, 0x4d, 0xe1, 0x70, 0x1b, 0x0a,
  0x14, 0x5a, 0x46, 0x00, 0x4a, 0xd0, 0x90, 0x33, 0x47, 0x80, 0x14, 0x03,
  0x10, 0x04, 0x01, 0x12, 0x28, 0x2a, 0x46, 0x23, 0x08, 0x02, 0x04, 0xd0,
  0x74, 0xd4, 0x70, 0xf9, 0x13, 0xf6, 0x10, 0x92, 0xcf, 0x6d, 0x54, 0xb1,
  0x12, 0x93, 0x8f, 0xdc, 0x36, 0x22, 0x04, 0x41, 0x10, 0x08, 0xba, 0x67,
  0xb8, 0xfc, 0x09, 0x7b, 0x08, 0xc9, 0x0f, 0x81, 0x66, 0x58, 0x08, 0x14,
  0x58, 0x85, 0xa8, 0x84, 0x4b, 0x20, 0xec, 0xa6, 0xe1, 0xf2, 0x27, 0xec,
  0x21, 0x24, 0x7f, 0x25, 0xa4, 0x95, 0x98, 0xfc, 0xe2, 0xb6, 0x51, 0x01,
  0x00, 0x00, 0x20, 0x14, 0x46, 0x13, 0xae, 0x0b, 0x00, 0x00, 0x40, 0x10,
  0x04, 0x80, 0xb6, 0x39, 0x82, 0xa0, 0x18, 0x97, 0x00, 0x09, 0x42, 0x47,
  0xde, 0x40, 0xc0, 0x30, 0x82, 0x00, 0x9c, 0x23, 0x4d, 0x11, 0x25, 0x4c,
  0x7e, 0xca, 0x8a, 0xcd, 0x43, 0x4d, 0x48, 0x08, 0xc2, 0x44, 0x10, 0x00,
  0x00, 0x00, 0x13, 0x14, 0x72, 0xc0, 0x87, 0x74, 0x60, 0x87, 0x36, 0x68,
  0x87, 0x79, 0x68, 0x03, 0x72, 0xc0, 0x87, 0x0d, 0xaf, 0x50, 0x0e, 0x6d,
  0xd0, 0x0e, 0x7a, 0x50, 0x0e, 0x6d, 0x00, 0x0f, 0x7a, 0x30, 0x07, 0x72,
  0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x71, 0xa0, 0x07, 0x73,
  0x20, 0x07, 0x6d, 0x90, 0x0e, 0x78, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d,
  0x90, 0x0e, 0x71, 0x60, 0x07, 0x7a, 0x30, 0x07, 0x72, 0xd0, 0x06, 0xe9,
  0x30, 0x07, 0x72, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x76,
  0x40, 0x07, 0x7a, 0x60, 0x07, 0x74, 0xd0, 0x06, 0xe6, 0x10, 0x07, 0x76,
  0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x60, 0x0e, 0x73, 0x20, 0x07, 0x7a,
  0x30, 0x07, 0x72, 0xd0, 0x06, 0xe6, 0x60, 0x07, 0x74, 0xa0, 0x07, 0x76,
  0x40, 0x07, 0x6d, 0xe0, 0x0e, 0x78, 0xa0, 0x07, 0x71, 0x60, 0x07, 0x7a,
  0x30, 0x07, 0x72, 0xa0, 0x07, 0x76, 0x40, 0x07, 0x3a, 0x0f, 0x84, 0x90,
  0x21, 0x23, 0x45, 0x44, 0x00, 0xd6, 0x00, 0x80, 0x79, 0x03, 0x00, 0xe6,
  0x0e, 0x00, 0x60, 0xc8, 0xe3, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xc0, 0x90, 0x27, 0x02, 0x02, 0x20, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x80, 0x21, 0xcf, 0x04, 0x04, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x1e, 0x0c, 0x08, 0x80, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x3c, 0x1b, 0x10, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x79, 0x3c, 0x20,
  0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc8, 0x02, 0x01,
  0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x32, 0x1e, 0x98, 0x18, 0x19, 0x11,
  0x4c, 0x90, 0x8c, 0x09, 0x26, 0x47, 0xc6, 0x04, 0x43, 0x82, 0x0a, 0x84,
  0x84, 0x12, 0x28, 0x85, 0x11, 0x80, 0x62, 0x28, 0xe4, 0x80, 0x1a, 0x28,
  0x82, 0x92, 0x28, 0x90, 0x42, 0x29, 0x83, 0x72, 0x28, 0x84, 0x82, 0x29,
  0x0f, 0x82, 0x0a, 0xa1, 0x08, 0x46, 0x00, 0x4a, 0x82, 0x94, 0x19, 0x00,
  0x5a, 0x66, 0x00, 0x88, 0x99, 0x01, 0x20, 0x64, 0x06, 0x80, 0x8c, 0x19,
  0x00, 0x22, 0x66, 0x00, 0x68, 0x98, 0x01, 0xa0, 0x60, 0x06, 0x80, 0xc4,
  0x19, 0x00, 0x1a, 0xc7, 0x42, 0x0c, 0x22, 0x10, 0x08, 0xe4, 0x79, 0x00,
  0x00, 0x00, 0x79, 0x18, 0x00, 0x00, 0xab, 0x00, 0x00, 0x00, 0x1a, 0x03,
  0x4c, 0x90, 0x46, 0x02, 0x13, 0xc4, 0x83, 0x0c, 0x6f, 0x0c, 0x24, 0xc6,
  0x45, 0x66, 0x43, 0x10, 0x4c, 0x10, 0x04, 0x65, 0x82, 0x20, 0x2c, 0x1b,
  0x84, 0x81, 0x98, 0x20, 0x08, 0xcc, 0x06, 0xc1, 0x30, 0x28, 0xc0, 0xcd,
  0x4d, 0x10, 0x84, 0x66, 0xc3, 0x80, 0x24, 0xc4, 0x04, 0xc1, 0x08, 0x03,
  0x3a, 0x54, 0x65, 0x78, 0x74, 0x75, 0x72, 0x65, 0x32, 0x44, 0x54, 0x61,
  0x62, 0x6c, 0x65, 0x13, 0x04, 0xc1, 0x99, 0x20, 0x08, 0xcf, 0x04, 0x41,
  0x80, 0x26, 0x08, 0x42, 0xb4, 0x41, 0x30, 0xa0, 0x0d, 0x89, 0xb1, 0x30,
  0x8d, 0xe1, 0x3c, 0x46, 0xb4, 0x21, 0x90, 0x26, 0x08, 0xcc, 0x47, 0xa7,
  0x29, 0x8c, 0xae, 0x4c, 0x2e, 0x2d, 0x8c, 0x2d, 0xc9, 0xcd, 0xec, 0x6d,
  0x48, 0x68, 0x82, 0x20, 0x48, 0x1b, 0x10, 0x83, 0xaa, 0x0c, 0x63, 0xb0,
  0x80, 0x0d, 0xc1, 0x35, 0x41, 0x40, 0xc0, 0x80, 0x0e, 0x51, 0x99, 0x59,
  0x58, 0x1d, 0x1b, 0xdd, 0x54, 0x58, 0x1b, 0x1c, 0x5b, 0x99, 0xdc, 0x06,
  0xc4, 0xc8, 0x34, 0xc3, 0x18, 0x0c, 0x60, 0x43, 0xb0, 0x6d, 0x20, 0x26,
  0x00, 0xe3, 0x26, 0x08, 0x84, 0x18, 0x4c, 0x10, 0x84, 0x89, 0x01, 0xda,
  0x04, 0x41, 0xa0, 0x26, 0x08, 0x42, 0xb5, 0xc1, 0x48, 0xc0, 0x20, 0x0c,
  0x0c, 0x31, 0x80, 0x48, 0xb4, 0xa5, 0xc1, 0xcd, 0x6d, 0x20, 0x12, 0x32,
  0x08, 0x03, 0x6b, 0x82, 0x10, 0x90, 0xc1, 0x06, 0xc1, 0x30, 0x83, 0x0d,
  0xc1, 0x19, 0x6c, 0x10, 0x0c, 0x34, 0xd8, 0x40, 0x7c, 0x63, 0x50, 0x06,
  0x69, 0x30, 0x41, 0x18, 0xc6, 0x60, 0x82, 0x20, 0x58, 0x34, 0xd0, 0xc2,
  0xdc, 0xc8, 0xd8, 0xca, 0x36, 0x18, 0x49, 0x1b, 0x84, 0x81, 0x21, 0x06,
  0xc4, 0x06, 0x81, 0x0d, 0xdc, 0x60, 0x82, 0xa0, 0x74, 0x94, 0xc2, 0xd8,
  0xc4, 0xca, 0xc8, 0xde, 0xa8, 0xca, 0xf0, 0xe8, 0xea, 0xe4, 0xca, 0x92,
  0xdc, 0xc8, 0xca, 0xf0, 0x36, 0x18, 0x49, 0x1c, 0x84, 0x81, 0x21, 0x06,
  0x04, 0x8f, 0xaf, 0xb2, 0x36, 0x3a, 0x38, 0xba, 0xbc, 0x0d, 0x46, 0x32,
  0x07, 0x61, 0xc0, 0x06, 0x62, 0x00, 0x6d, 0x18, 0x2c, 0x39, 0xa0, 0x83,
  0x09, 0x82, 0x18, 0x94, 0xc1, 0x04, 0x41, 0xb8, 0x68, 0x0c, 0xbd, 0xb1,
  0xbd, 0xd5, 0xc9, 0xd1, 0x18, 0x7a, 0x62, 0x7a, 0xaa, 0x92, 0xda, 0x80,
  0x24, 0x78, 0x10, 0x06, 0x06, 0x1b, 0xe4, 0x81, 0x18, 0x40, 0x44, 0xa8,
  0xca, 0xf0, 0x86, 0xde, 0xde, 0xe4, 0xc8, 0x88, 0x50, 0x15, 0x61, 0x0d,
  0x3d, 0x3d, 0x49, 0x11, 0x6d, 0x40, 0x92, 0x3d, 0x08, 0x03, 0x8b, 0x0d,
  0xf8, 0x40, 0x0c, 0xa0, 0x0d, 0xc3, 0x1d, 0xe8, 0x41, 0x1f, 0x4c, 0x10,
  0x16, 0x6f, 0x03, 0x91, 0x54, 0x61, 0x60, 0x6c, 0x10, 0x2c, 0x50, 0xd8,
  0xb0, 0x18, 0x9e, 0x1a, 0xac, 0xc1, 0x1b, 0xc0, 0x41, 0x1d, 0xd8, 0x81,
  0x1f, 0xfc, 0x41, 0x28, 0x4c, 0x10, 0x9e, 0x61, 0x03, 0xb0, 0x61, 0x30,
  0x48, 0x81, 0x14, 0x36, 0x04, 0xa5, 0xb0, 0x61, 0x18, 0x46, 0xc1, 0x14,
  0x26, 0x08, 0x63, 0x60, 0x06, 0x1b, 0x02, 0x54, 0x20, 0xd1, 0x16, 0x96,
  0xe6, 0x36, 0x41, 0x80, 0xb8, 0x09, 0x02, 0xb4, 0x6d, 0x08, 0x8c, 0x09,
  0x02, 0xa4, 0x4d, 0x10, 0xa0, 0x6c, 0x82, 0x20, 0x60, 0x1b, 0x84, 0x30,
  0x80, 0x85, 0x0d, 0x8b, 0x91, 0x07, 0xab, 0xc0, 0x0a, 0xad, 0xe0, 0x0a,
  0xc3, 0x2b, 0x18, 0xac, 0x10, 0x0b, 0x1b, 0x84, 0x30, 0x08, 0x83, 0x0d,
  0xcb, 0xc0, 0x07, 0xab, 0xc0, 0x0a, 0xad, 0xe0, 0x0a, 0x83, 0x2b, 0x0c,
  0xac, 0x30, 0x0b, 0x1b, 0x04, 0x59, 0xa0, 0x05, 0x26, 0x53, 0x56, 0x5f,
  0x54, 0x61, 0x72, 0x67, 0x65, 0x74, 0x13, 0x04, 0x28, 0xd9, 0xb0, 0x18,
  0xb6, 0xb0, 0x0a, 0xb7, 0xd0, 0x0a, 0xac, 0x30, 0xbc, 0x82, 0xc1, 0x0a,
  0xb1, 0xb0, 0x21, 0xc0, 0x85, 0x0d, 0x43, 0x2d, 0xe4, 0x02, 0xb0, 0xa1,
  0x18, 0x05, 0x55, 0xd0, 0x85, 0x0e, 0xa8, 0xc2, 0xc6, 0x66, 0xd7, 0xe6,
  0x92, 0x46, 0x56, 0xe6, 0x46, 0x37, 0x25, 0x08, 0xaa, 0x90, 0xe1, 0xb9,
  0xd8, 0x95, 0xc9, 0xcd, 0xa5, 0xbd, 0xb9, 0x4d, 0x09, 0x88, 0x26, 0x64,
  0x78, 0x2e, 0x76, 0x61, 0x6c, 0x76, 0x65, 0x72, 0x53, 0x02, 0xa3, 0x0e,
  0x19, 0x9e, 0xcb, 0x1c, 0x5a, 0x18, 0x59, 0x99, 0x5c, 0xd3, 0x1b, 0x59,
  0x19, 0xdb, 0x94, 0x20, 0x29, 0x43, 0x86, 0xe7, 0x22, 0x57, 0x36, 0xf7,
  0x56, 0x27, 0x37, 0x56, 0x36, 0x37, 0x25, 0xe0, 0x2a, 0x91, 0xe1, 0xb9,
  0xd0, 0xe5, 0xc1, 0x95, 0x05, 0xb9, 0xb9, 0xbd, 0xd1, 0x85, 0xd1, 0xa5,
  0xbd, 0xb9, 0xcd, 0x4d, 0x11, 0x42, 0xc1, 0x14, 0xea, 0x90, 0xe1, 0xb9,
  0xd8, 0xa5, 0x95, 0xdd, 0x25, 0x91, 0x4d, 0xd1, 0x85, 0xd1, 0x95, 0x4d,
  0x09, 0x50, 0xa1, 0x0e, 0x19, 0x9e, 0x4b, 0x99, 0x1b, 0x9d, 0x5c, 0x1e,
  0xd4, 0x5b, 0x9a, 0x1b, 0xdd, 0xdc, 0x94, 0x40, 0x17, 0x00, 0x79, 0x18,
  0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x33, 0x08, 0x80, 0x1c, 0xc4, 0xe1,
  0x1c, 0x66, 0x14, 0x01, 0x3d, 0x88, 0x43, 0x38, 0x84, 0xc3, 0x8c, 0x42,
  0x80, 0x07, 0x79, 0x78, 0x07, 0x73, 0x98, 0x71, 0x0c, 0xe6, 0x00, 0x0f,
  0xed, 0x10, 0x0e, 0xf4, 0x80, 0x0e, 0x33, 0x0c, 0x42, 0x1e, 0xc2, 0xc1,
  0x1d, 0xce, 0xa1, 0x1c, 0x66, 0x30, 0x05, 0x3d, 0x88, 0x43, 0x38, 0x84,
  0x83, 0x1b, 0xcc, 0x03, 0x3d, 0xc8, 0x43, 0x3d, 0x8c, 0x03, 0x3d, 0xcc,
  0x78, 0x8c, 0x74, 0x70, 0x07, 0x7b, 0x08, 0x07, 0x79, 0x48, 0x87, 0x70,
  0x70, 0x07, 0x7a, 0x70, 0x03, 0x76, 0x78, 0x87, 0x70, 0x20, 0x87, 0x19,
  0xcc, 0x11, 0x0e, 0xec, 0x90, 0x0e, 0xe1, 0x30, 0x0f, 0x6e, 0x30, 0x0f,
  0xe3, 0xf0, 0x0e, 0xf0, 0x50, 0x0e, 0x33, 0x10, 0xc4, 0x1d, 0xde, 0x21,
  0x1c, 0xd8, 0x21, 0x1d, 0xc2, 0x61, 0x1e, 0x66, 0x30, 0x89, 0x3b, 0xbc,
  0x83, 0x3b, 0xd0, 0x43, 0x39, 0xb4, 0x03, 0x3c, 0xbc, 0x83, 0x3c, 0x84,
  0x03, 0x3b, 0xcc, 0xf0, 0x14, 0x76, 0x60, 0x07, 0x7b, 0x68, 0x07, 0x37,
  0x68, 0x87, 0x72, 0x68, 0x07, 0x37, 0x80, 0x87, 0x70, 0x90, 0x87, 0x70,
  0x60, 0x07, 0x76, 0x28, 0x07, 0x76, 0xf8, 0x05, 0x76, 0x78, 0x87, 0x77,
  0x80, 0x87, 0x5f, 0x08, 0x87, 0x71, 0x18, 0x87, 0x72, 0x98, 0x87, 0x79,
  0x98, 0x81, 0x2c, 0xee, 0xf0, 0x0e, 0xee, 0xe0, 0x0e, 0xf5, 0xc0, 0x0e,
  0xec, 0x30, 0x03, 0x62, 0xc8, 0xa1, 0x1c, 0xe4, 0xa1, 0x1c, 0xcc, 0xa1,
  0x1c, 0xe4, 0xa1, 0x1c, 0xdc, 0x61, 0x1c, 0xca, 0x21, 0x1c, 0xc4, 0x81,
  0x1d, 0xca, 0x61, 0x06, 0xd6, 0x90, 0x43, 0x39, 0xc8, 0x43, 0x39, 0x98,
  0x43, 0x39, 0xc8, 0x43, 0x39, 0xb8, 0xc3, 0x38, 0x94, 0x43, 0x38, 0x88,
  0x03, 0x3b, 0x94, 0xc3, 0x2f, 0xbc, 0x83, 0x3c, 0xfc, 0x82, 0x3b, 0xd4,
  0x03, 0x3b, 0xb0, 0xc3, 0x8c, 0xcc, 0x21, 0x07, 0x7c, 0x70, 0x03, 0x74,
  0x60, 0x07, 0x37, 0x90, 0x87, 0x72, 0x98, 0x87, 0x77, 0xa8, 0x07, 0x79,
  0x18, 0x87, 0x72, 0x70, 0x83, 0x70, 0xa0, 0x07, 0x7a, 0x90, 0x87, 0x74,
  0x10, 0x87, 0x7a, 0xa0, 0x87, 0x72, 0x00, 0x00, 0x00, 0x00, 0x71, 0x20,
  0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x15, 0x30, 0x06, 0x81, 0x1f, 0xb1,
  0x6c, 0x0e, 0xd7, 0xd9, 0xf4, 0x69, 0xb8, 0x0d, 0x67, 0x97, 0xe5, 0x40,
  0xe0, 0xac, 0x3a, 0x0d, 0xb7, 0xe1, 0xec, 0xb2, 0x7c, 0x4a, 0x0f, 0xd3,
  0xcb, 0x40, 0x60, 0xb0, 0x00, 0xea, 0x20, 0xf0, 0xa3, 0x96, 0xf1, 0xf4,
  0xba, 0xbc, 0x2c, 0x23, 0x52, 0xc3, 0x62, 0x76, 0x19, 0x08, 0x9c, 0x41,
  0x83, 0xd6, 0x1f, 0x89, 0x5a, 0xc6, 0xd3, 0xeb, 0xf2, 0xb2, 0x8c, 0x08,
  0xb4, 0xfe, 0x48, 0xf6, 0xf2, 0x98, 0xfe, 0x96, 0x03, 0x9b, 0x24, 0xd8,
  0x0c, 0x08, 0x04, 0x02, 0x83, 0x26, 0xe0, 0x98, 0xc0, 0x44, 0x44, 0x08,
  0xb0, 0x88, 0x4d, 0xe1, 0x70, 0x9b, 0x19, 0x6c, 0xc3, 0xe5, 0x3b, 0x8f,
  0x2f, 0x04, 0x54, 0x51, 0x10, 0x51, 0xe9, 0x00, 0x43, 0x49, 0x18, 0x80,
  0x80, 0xf9, 0xc8, 0x6d, 0x1b, 0x82, 0x34, 0x5c, 0xbe, 0xf3, 0xf8, 0x42,
  0x44, 0x00, 0x13, 0x11, 0x02, 0xcd, 0xb0, 0x10, 0x46, 0x30, 0x0d, 0x97,
  0xef, 0x3c, 0xfe, 0xe2, 0x00, 0x83, 0xd8, 0x3c, 0xd4, 0xe4, 0x17, 0xb7,
  0x6d, 0x07, 0xd0, 0x70, 0xf9, 0xce, 0xe3, 0x4b, 0x00, 0xf3, 0x2c, 0x84,
  0x5f, 0xdc, 0xb6, 0x15, 0x54, 0xc3, 0xe5, 0x3b, 0x8f, 0x2f, 0x4d, 0x4e,
  0x44, 0xa0, 0xd4, 0xf4, 0x50, 0x93, 0x5f, 0xdc, 0xb6, 0x0d, 0x10, 0x0c,
  0x80, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x41,
  0x53, 0x48, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0xcf,
  0x8f, 0xd6, 0x87, 0x24, 0x05, 0x32, 0x7f, 0xae, 0x62, 0x4c, 0x98, 0xc9,
  0x69, 0xfd, 0x44, 0x58, 0x49, 0x4c, 0x88, 0x07, 0x00, 0x00, 0x65, 0x00,
  0x00, 0x00, 0xe2, 0x01, 0x00, 0x00, 0x44, 0x58, 0x49, 0x4c, 0x05, 0x01,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x70, 0x07, 0x00, 0x00, 0x42, 0x43,
  0xc0, 0xde, 0x21, 0x0c, 0x00, 0x00, 0xd9, 0x01, 0x00, 0x00, 0x0b, 0x82,
  0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x07, 0x81,
  0x23, 0x91, 0x41, 0xc8, 0x04, 0x49, 0x06, 0x10, 0x32, 0x39, 0x92, 0x01,
  0x84, 0x0c, 0x25, 0x05, 0x08, 0x19, 0x1e, 0x04, 0x8b, 0x62, 0x80, 0x18,
  0x45, 0x02, 0x42, 0x92, 0x0b, 0x42, 0xc4, 0x10, 0x32, 0x14, 0x38, 0x08,
  0x18, 0x4b, 0x0a, 0x32, 0x62, 0x88, 0x48, 0x90, 0x14, 0x20, 0x43, 0x46,
  0x88, 0xa5, 0x00, 0x19, 0x32, 0x42, 0xe4, 0x48, 0x0e, 0x90, 0x11, 0x23,
  0xc4, 0x50, 0x41, 0x51, 0x81, 0x8c, 0xe1, 0x83, 0xe5, 0x8a, 0x04, 0x31,
  0x46, 0x06, 0x51, 0x18, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1b, 0x8c,
  0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x40, 0x02, 0xa8, 0x0d, 0x84, 0xf0,
  0xff, 0xff, 0xff, 0xff, 0x03, 0x20, 0x6d, 0x30, 0x86, 0xff, 0xff, 0xff,
  0xff, 0x1f, 0x00, 0x09, 0xa8, 0x00, 0x49, 0x18, 0x00, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x13, 0x82, 0x60, 0x42, 0x20, 0x4c, 0x08, 0x06, 0x00, 0x00,
  0x00, 0x00, 0x89, 0x20, 0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 0x32, 0x22,
  0x88, 0x09, 0x20, 0x64, 0x85, 0x04, 0x13, 0x23, 0xa4, 0x84, 0x04, 0x13,
  0x23, 0xe3, 0x84, 0xa1, 0x90, 0x14, 0x12, 0x4c, 0x8c, 0x8c, 0x0b, 0x84,
  0xc4, 0x4c, 0x10, 0x8c, 0xc1, 0x08, 0x40, 0x09, 0x00, 0x0a, 0x66, 0x00,
  0xe6, 0x08, 0xc0, 0x60, 0x8e, 0x00, 0x29, 0xc6, 0x40, 0x10, 0x44, 0x41,
  0x90, 0x51, 0x0c, 0x80, 0x20, 0x88, 0x62, 0x20, 0xe4, 0xa8, 0xe1, 0xf2,
  0x27, 0xec, 0x21, 0x24, 0x9f, 0xdb, 0xa8, 0x62, 0x25, 0x26, 0x1f, 0xb9,
  0x6d, 0x44, 0x10, 0x04, 0x41, 0x50, 0x71, 0xcf, 0x70, 0xf9, 0x13, 0xf6,
  0x10, 0x92, 0x1f, 0x02, 0xcd, 0xb0, 0x10, 0x28, 0x58, 0x0a, 0xa1, 0x10,
  0x0c, 0x41, 0xcd, 0x4d, 0xc3, 0xe5, 0x4f, 0xd8, 0x43, 0x48, 0xfe, 0x4a,
  0x48, 0x2b, 0x31, 0xf9, 0xc5, 0x6d, 0xa3, 0x62, 0x18, 0x86, 0x81, 0x28,
  0xcc, 0x43, 0x30, 0xcc, 0x30, 0x0c, 0x03, 0x41, 0x10, 0x03, 0x41, 0x73,
  0x04, 0x41, 0x31, 0x18, 0xa2, 0x20, 0x08, 0x89, 0xa6, 0x81, 0x80, 0x61,
  0x04, 0x62, 0x98, 0xa9, 0x0d, 0xc6, 0x81, 0x1d, 0xc2, 0x61, 0x1e, 0xe6,
  0xc1, 0x0d, 0x68, 0xa1, 0x1c, 0xf0, 0x81, 0x1e, 0xea, 0x41, 0x1e, 0xca,
  0x41, 0x0e, 0x48, 0x81, 0x0f, 0xec, 0xa1, 0x1c, 0xc6, 0x81, 0x1e, 0xde,
  0x41, 0x1e, 0xf8, 0xc0, 0x1c, 0xd8, 0xe1, 0x1d, 0xc2, 0x81, 0x1e, 0xd8,
  0x00, 0x0c, 0xe8, 0xc0, 0x0f, 0xc0, 0xc0, 0x0f, 0xf4, 0x40, 0x0f, 0xda,
  0x21, 0x1d, 0xe0, 0x61, 0x1e, 0x7e, 0x81, 0x1e, 0xf2, 0x01, 0x1e, 0xca,
  0x01, 0x05, 0xc4, 0x4c, 0x62, 0x30, 0x0e, 0xec, 0x10, 0x0e, 0xf3, 0x30,
  0x0f, 0x6e, 0x40, 0x0b, 0xe5, 0x80, 0x0f, 0xf4, 0x50, 0x0f, 0xf2, 0x50,
  0x0e, 0x72, 0x40, 0x0a, 0x7c, 0x60, 0x0f, 0xe5, 0x30, 0x0e, 0xf4, 0xf0,
  0x0e, 0xf2, 0xc0, 0x07, 0xe6, 0xc0, 0x0e, 0xef, 0x10, 0x0e, 0xf4, 0xc0,
  0x06, 0x60, 0x40, 0x07, 0x7e, 0x00, 0x06, 0x7e, 0x80, 0x04, 0xeb, 0x4a,
  0x00, 0x8c, 0xb2, 0x61, 0x84, 0x61, 0xb8, 0x49, 0x9a, 0x22, 0x4a, 0x98,
  0x7c, 0x13, 0x98, 0x88, 0x08, 0x01, 0x16, 0xb1, 0x29, 0x1c, 0x24, 0x10,
  0xdb, 0x39, 0x26, 0x30, 0x11, 0x11, 0x02, 0x2c, 0x62, 0x53, 0x38, 0xdc,
  0x86, 0x02, 0x8e, 0xba, 0x9b, 0xa4, 0x29, 0xa2, 0x84, 0xc9, 0x67, 0x01,
  0xe6, 0x59, 0x88, 0x88, 0x9d, 0x80, 0x89, 0x40, 0x01, 0x41, 0x5f, 0x22,
  0x10, 0x53, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x72, 0xc0, 0x87, 0x74,
  0x60, 0x87, 0x36, 0x68, 0x87, 0x79, 0x68, 0x03, 0x72, 0xc0, 0x87, 0x0d,
  0xaf, 0x50, 0x0e, 0x6d, 0xd0, 0x0e, 0x7a, 0x50, 0x0e, 0x6d, 0x00, 0x0f,
  0x7a, 0x30, 0x07, 0x72, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e,
  0x71, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x78, 0xa0, 0x07,
  0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x71, 0x60, 0x07, 0x7a, 0x30, 0x07,
  0x72, 0xd0, 0x06, 0xe9, 0x30, 0x07, 0x72, 0xa0, 0x07, 0x73, 0x20, 0x07,
  0x6d, 0x90, 0x0e, 0x76, 0x40, 0x07, 0x7a, 0x60, 0x07, 0x74, 0xd0, 0x06,
  0xe6, 0x10, 0x07, 0x76, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x60, 0x0e,
  0x73, 0x20, 0x07, 0x7a, 0x30, 0x07, 0x72, 0xd0, 0x06, 0xe6, 0x60, 0x07,
  0x74, 0xa0, 0x07, 0x76, 0x40, 0x07, 0x6d, 0xe0, 0x0e, 0x78, 0xa0, 0x07,
  0x71, 0x60, 0x07, 0x7a, 0x30, 0x07, 0x72, 0xa0, 0x07, 0x76, 0x40, 0x07,
  0x43, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x86, 0x3c, 0x06, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0c, 0x79, 0x10, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x18, 0xf2, 0x34, 0x40, 0x00, 0x0c, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0xe4, 0x81, 0x80, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xc8, 0x33, 0x01, 0x01, 0x30, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x16, 0x08, 0x00, 0x10, 0x00,
  0x00, 0x00, 0x32, 0x1e, 0x98, 0x14, 0x19, 0x11, 0x4c, 0x90, 0x8c, 0x09,
  0x26, 0x47, 0xc6, 0x04, 0x43, 0x22, 0x4a, 0xa0, 0x14, 0x8a, 0x61, 0x04,
  0xa0, 0x90, 0x03, 0x6a, 0xa0, 0x08, 0x4a, 0xa2, 0x40, 0xca, 0xa0, 0x3c,
  0xa8, 0x28, 0x89, 0x11, 0x80, 0x22, 0x28, 0x84, 0x02, 0x21, 0x6d, 0x06,
  0x80, 0xbc, 0x19, 0x00, 0x02, 0x67, 0x00, 0x28, 0x1c, 0x0b, 0x31, 0x88,
  0x40, 0x20, 0x90, 0xe7, 0x01, 0x00, 0x79, 0x18, 0x00, 0x00, 0x5c, 0x00,
  0x00, 0x00, 0x1a, 0x03, 0x4c, 0x90, 0x46, 0x02, 0x13, 0xc4, 0x83, 0x0c,
  0x6f, 0x0c, 0x24, 0xc6, 0x45, 0x66, 0x43, 0x10, 0x4c, 0x10, 0x08, 0x63,
  0x82, 0x40, 0x1c, 0x1b, 0x84, 0x81, 0xa0, 0x00, 0x37, 0x37, 0x41, 0x20,
  0x90, 0x0d, 0x83, 0x71, 0x10, 0x13, 0x04, 0x22, 0x99, 0x20, 0x68, 0x16,
  0x81, 0x09, 0x02, 0xa1, 0x4c, 0x10, 0x88, 0x65, 0x82, 0x40, 0x30, 0x13,
  0x04, 0xa2, 0xd9, 0x20, 0x24, 0xcf, 0x86, 0x24, 0x51, 0x16, 0x26, 0x69,
  0x9c, 0x04, 0xda, 0x10, 0x44, 0x13, 0x04, 0xef, 0x9a, 0x20, 0x10, 0xce,
  0x06, 0x24, 0x99, 0x96, 0x24, 0x19, 0x28, 0x60, 0x43, 0x50, 0x4d, 0x10,
  0xc0, 0x00, 0xdb, 0x80, 0x24, 0xd7, 0x92, 0x24, 0x43, 0x02, 0x6c, 0x08,
  0xb0, 0x0d, 0x84, 0x04, 0x58, 0xd9, 0x04, 0x21, 0x0c, 0xb2, 0x0d, 0xc1,
  0x36, 0x41, 0x10, 0x00, 0x12, 0x6d, 0x61, 0x69, 0x6e, 0x34, 0x86, 0x9e,
  0x98, 0x9e, 0xaa, 0xa4, 0x26, 0x08, 0x45, 0x34, 0x41, 0x28, 0xa4, 0x0d,
  0x41, 0x32, 0x41, 0x28, 0xa6, 0x09, 0x42, 0x41, 0x4d, 0x10, 0x88, 0x67,
  0x82, 0x40, 0x40, 0x1b, 0x84, 0x32, 0x30, 0x83, 0x0d, 0x4b, 0xf2, 0x81,
  0x41, 0x18, 0x88, 0xc1, 0x18, 0x0c, 0x64, 0x90, 0x84, 0xc1, 0x19, 0x10,
  0xa1, 0x2a, 0xc2, 0x1a, 0x7a, 0x7a, 0x92, 0x22, 0xda, 0x20, 0x94, 0x41,
  0x19, 0x6c, 0x58, 0x86, 0x34, 0x00, 0x83, 0x30, 0x10, 0x83, 0x31, 0x18,
  0xc6, 0x60, 0x08, 0x03, 0x35, 0xd8, 0x20, 0xa0, 0xc1, 0x1a, 0x30, 0x99,
  0xb2, 0xfa, 0xa2, 0x0a, 0x93, 0x3b, 0x2b, 0xa3, 0x9b, 0x20, 0x14, 0xd5,
  0x86, 0x25, 0x69, 0x03, 0x30, 0x70, 0x03, 0x31, 0x08, 0x83, 0x81, 0x0c,
  0x92, 0x30, 0x38, 0x83, 0x0d, 0xc1, 0x1b, 0x6c, 0x18, 0xd8, 0x00, 0x0e,
  0x80, 0x0d, 0x45, 0xe7, 0xc5, 0x81, 0x06, 0x54, 0x61, 0x63, 0xb3, 0x6b,
  0x73, 0x49, 0x23, 0x2b, 0x73, 0xa3, 0x9b, 0x12, 0x04, 0x55, 0xc8, 0xf0,
  0x5c, 0xec, 0xca, 0xe4, 0xe6, 0xd2, 0xde, 0xdc, 0xa6, 0x04, 0x44, 0x13,
  0x32, 0x3c, 0x17, 0xbb, 0x30, 0x36, 0xbb, 0x32, 0xb9, 0x29, 0x01, 0x51,
  0x87, 0x0c, 0xcf, 0x65, 0x0e, 0x2d, 0x8c, 0xac, 0x4c, 0xae, 0xe9, 0x8d,
  0xac, 0x8c, 0x6d, 0x4a, 0x70, 0x94, 0x21, 0xc3, 0x73, 0x91, 0x2b, 0x9b,
  0x7b, 0xab, 0x93, 0x1b, 0x2b, 0x9b, 0x9b, 0x12, 0x64, 0x75, 0xc8, 0xf0,
  0x5c, 0xec, 0xd2, 0xca, 0xee, 0x92, 0xc8, 0xa6, 0xe8, 0xc2, 0xe8, 0xca,
  0xa6, 0x04, 0x5b, 0x1d, 0x32, 0x3c, 0x97, 0x32, 0x37, 0x3a, 0xb9, 0x3c,
  0xa8, 0xb7, 0x34, 0x37, 0xba, 0xb9, 0x29, 0x41, 0x1c, 0x00, 0x79, 0x18,
  0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x33, 0x08, 0x80, 0x1c, 0xc4, 0xe1,
  0x1c, 0x66, 0x14, 0x01, 0x3d, 0x88, 0x43, 0x38, 0x84, 0xc3, 0x8c, 0x42,
  0x80, 0x07, 0x79, 0x78, 0x07, 0x73, 0x98, 0x71, 0x0c, 0xe6, 0x00, 0x0f,
  0xed, 0x10, 0x0e, 0xf4, 0x80, 0x0e, 0x33, 0x0c, 0x42, 0x1e, 0xc2, 0xc1,
  0x1d, 0xce, 0xa1, 0x1c, 0x66, 0x30, 0x05, 0x3d, 0x88, 0x43, 0x38, 0x84,
  0x83, 0x1b, 0xcc, 0x03, 0x3d, 0xc8, 0x43, 0x3d, 0x8c, 0x03, 0x3d, 0xcc,
  0x78, 0x8c, 0x74, 0x70, 0x07, 0x7b, 0x08, 0x07, 0x79, 0x48, 0x87, 0x70,
  0x70, 0x07, 0x7a, 0x70, 0x03, 0x76, 0x78, 0x87, 0x70, 0x20, 0x87, 0x19,
  0xcc, 0x11, 0x0e, 0xec, 0x90, 0x0e, 0xe1, 0x30, 0x0f, 0x6e, 0x30, 0x0f,
  0xe3, 0xf0, 0x0e, 0xf0, 0x50, 0x0e, 0x33, 0x10, 0xc4, 0x1d, 0xde, 0x21,
  0x1c, 0xd8, 0x21, 0x1d, 0xc2, 0x61, 0x1e, 0x66, 0x30, 0x89, 0x3b, 0xbc,
  0x83, 0x3b, 0xd0, 0x43, 0x39, 0xb4, 0x03, 0x3c, 0xbc, 0x83, 0x3c, 0x84,
  0x03, 0x3b, 0xcc, 0xf0, 0x14, 0x76, 0x60, 0x07, 0x7b, 0x68, 0x07, 0x37,
  0x68, 0x87, 0x72, 0x68, 0x07, 0x37, 0x80, 0x87, 0x70, 0x90, 0x87, 0x70,
  0x60, 0x07, 0x76, 0x28, 0x07, 0x76, 0xf8, 0x05, 0x76, 0x78, 0x87, 0x77,
  0x80, 0x87, 0x5f, 0x08, 0x87, 0x71, 0x18, 0x87, 0x72, 0x98, 0x87, 0x79,
  0x98, 0x81, 0x2c, 0xee, 0xf0, 0x0e, 0xee, 0xe0, 0x0e, 0xf5, 0xc0, 0x0e,
  0xec, 0x30, 0x03, 0x62, 0xc8, 0xa1, 0x1c, 0xe4, 0xa1, 0x1c, 0xcc, 0xa1,
  0x1c, 0xe4, 0xa1, 0x1c, 0xdc, 0x61, 0x1c, 0xca, 0x21, 0x1c, 0xc4, 0x81,
  0x1d, 0xca, 0x61, 0x06, 0xd6, 0x90, 0x43, 0x39, 0xc8, 0x43, 0x39, 0x98,
  0x43, 0x39, 0xc8, 0x43, 0x39, 0xb8, 0xc3, 0x38, 0x94, 0x43, 0x38, 0x88,
  0x03, 0x3b, 0x94, 0xc3, 0x2f, 0xbc, 0x83, 0x3c, 0xfc, 0x82, 0x3b, 0xd4,
  0x03, 0x3b, 0xb0, 0xc3, 0x8c, 0xcc, 0x21, 0x07, 0x7c, 0x70, 0x03, 0x74,
  0x60, 0x07, 0x37, 0x90, 0x87, 0x72, 0x98, 0x87, 0x77, 0xa8, 0x07, 0x79,
  0x18, 0x87, 0x72, 0x70, 0x83, 0x70, 0xa0, 0x07, 0x7a, 0x90, 0x87, 0x74,
  0x10, 0x87, 0x7a, 0xa0, 0x87, 0x72, 0x00, 0x00, 0x00, 0x00, 0x71, 0x20,
  0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x36, 0xb0, 0x0d, 0x97, 0xef, 0x3c,
  0xbe, 0x10, 0x50, 0x45, 0x41, 0x44, 0xa5, 0x03, 0x0c, 0x25, 0x61, 0x00,
  0x02, 0xe6, 0x23, 0xb7, 0x6d, 0x05, 0xd2, 0x70, 0xf9, 0xce, 0xe3, 0x0b,
  0x11, 0x01, 0x4c, 0x44, 0x08, 0x34, 0xc3, 0x42, 0x58, 0xc0, 0x34, 0x5c,
  0xbe, 0xf3, 0xf8, 0x8b, 0x03, 0x0c, 0x62, 0xf3, 0x50, 0x93, 0x5f, 0xdc,
  0xb6, 0x11, 0x40, 0xc3, 0xe5, 0x3b, 0x8f, 0x2f, 0x01, 0xcc, 0xb3, 0x10,
  0x7e, 0x71, 0xdb, 0x26, 0x50, 0x0d, 0x97, 0xef, 0x3c, 0xbe, 0x34, 0x39,
  0x11, 0x81, 0x52, 0xd3, 0x43, 0x4d, 0x7e, 0x71, 0xdb, 0x06, 0x40, 0x30,
  0x00, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x61, 0x20, 0x00, 0x00, 0x4b, 0x00,
  0x00, 0x00, 0x13, 0x04, 0x43, 0x2c, 0x10, 0x00, 0x00, 0x00, 0x06, 0x00,
  0x00, 0x00, 0x24, 0x8d, 0x00, 0x10, 0x31, 0x03, 0x50, 0x08, 0x25, 0x57,
  0x76, 0x85, 0x47, 0x45, 0x19, 0x94, 0x00, 0x0d, 0x33, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x23, 0x06, 0x09, 0x00, 0x82, 0x60, 0x30, 0x79, 0xc6, 0xa0,
  0x69, 0xc9, 0x88, 0x41, 0x02, 0x80, 0x20, 0x18, 0x4c, 0xdf, 0x11, 0x6d,
  0x9b, 0x32, 0x62, 0x90, 0x00, 0x20, 0x08, 0x06, 0x06, 0x19, 0x24, 0x1f,
  0x37, 0x29, 0x23, 0x06, 0x09, 0x00, 0x82, 0x60, 0x60, 0x94, 0x81, 0x02,
  0x06, 0x5d, 0xb1, 0x8c, 0x18, 0x24, 0x00, 0x08, 0x82, 0x81, 0x61, 0x06,
  0x8b, 0xe7, 0x55, 0xcc, 0x88, 0x41, 0x02, 0x80, 0x20, 0x18, 0x18, 0x67,
  0xc0, 0x7c, 0xdf, 0xd1, 0x8c, 0x18, 0x24, 0x00, 0x08, 0x82, 0x81, 0x81,
  0x06, 0x0d, 0x18, 0x80, 0x81, 0xe5, 0x8c, 0x18, 0x24, 0x00, 0x08, 0x82,
  0x81, 0x91, 0x06, 0x4e, 0x18, 0x84, 0x81, 0xf2, 0x8c, 0x18, 0x1c, 0x00,
  0x08, 0x82, 0x41, 0x83, 0x06, 0xcd, 0x21, 0x06, 0xa3, 0x09, 0x01, 0x30,
  0xdc, 0x10, 0x88, 0x01, 0x18, 0xcc, 0x32, 0x08, 0x41, 0x50, 0x42, 0x19,
  0xc0, 0x88, 0x41, 0x02, 0x80, 0x20, 0x18, 0x4c, 0x6a, 0x20, 0x75, 0x66,
  0x10, 0x54, 0x23, 0x06, 0x0f, 0x00, 0x82, 0x60, 0x00, 0xb1, 0x41, 0x14,
  0x34, 0x8b, 0xe2, 0x38, 0x67, 0x70, 0x06, 0x95, 0x33, 0x9a, 0x10, 0x00,
  0xa3, 0x09, 0x42, 0x30, 0x9a, 0x30, 0x08, 0xa3, 0x09, 0xc4, 0x30, 0x4b,
  0x20, 0x0c, 0x54, 0x0c, 0x48, 0xc0, 0x01, 0x03, 0x15, 0x03, 0x12, 0x70,
  0xc0, 0x40, 0xc5, 0x80, 0x04, 0x1c, 0x30, 0x50, 0x31, 0x20, 0x01, 0x07,
  0x8c, 0x18, 0x24, 0x00, 0x08, 0x82, 0x01, 0x72, 0x07, 0x72, 0x00, 0x07,
  0x70, 0x70, 0x06, 0xc4, 0x88, 0x41, 0x02, 0x80, 0x20, 0x18, 0x20, 0x77,
  0x20, 0x07, 0x70, 0x00, 0x07, 0xd8, 0x30, 0x62, 0x90, 0x00, 0x20, 0x08,
  0x06, 0xc8, 0x1d, 0xc8, 0x01, 0x1c, 0xc0, 0x81, 0x19, 0x08, 0x23, 0x06,
  0x09, 0x00, 0x82, 0x60, 0x80, 0xdc, 0x81, 0x1c, 0xc0, 0x01, 0x1c, 0x64,
  0x01, 0x02, 0x00, 0x00, 0x00, 0x00
};
