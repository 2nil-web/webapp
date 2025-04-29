function VBArgs ()
  redim vargs(WScript.Arguments.Count-1)
  dim i
  for i=0 to WScript.Arguments.Count-1
    vargs(i)=WScript.Arguments(i)
  next
  VBArgs=vargs
end function

