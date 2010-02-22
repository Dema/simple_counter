module Main where
import Language.Atom

main = compileExample

-- | Invoke the atom compiler.
compileExample :: IO ()
compileExample = do
  (schedule, _, _, _, _) <- compile "counter" defaults { cCode = prePostCode } counter
  putStrLn $ reportSchedule schedule

prePostCode :: [Name] -> [Name] -> [(Name, Type)] -> (String, String)
prePostCode _ _ _ =
  ( unlines
    [ "#include <stdlib.h>"
    , "#include <stdio.h>",
    "void updateIndicator(){}",
    "unsigned char plus_on = 0;",
    "unsigned char minus_on = 0;",
    "unsigned int number = 1;"
    ]
  , unlines
    [ "ISR(){",
        " counter();",
        " }",
      "void main(int argc, char* argv[]) {"
    , "}"
    ]
  )

counter :: Atom ()
counter = do

    let plusOn' = bool' "plus_on"
    let minusOn' = bool' "minus_on"
    let number = word16' "number"

    checkButton "plus" number (+) plusOn'
    checkButton "minus" number (-) minusOn'

    period 5 $ atom "update_indicator" $ do
        updateIndicator

checkButton name number operation variable = do
    holdTimer <- timer $ "hold" ++ name
    debouncedVar <- debounce (name ++ "_debounce") 30 30 False $ value variable
    period 50 $ atom ("check_" ++ name) $ phase 2 $ do
        cond $ debouncedVar
        number <== value number `operation` 1
        startTimer holdTimer 500
    period 50 $ atom ("check_" ++ name ++ "_hold") $ phase 1 $ do
        cond $ debouncedVar &&. timerDone holdTimer
        number <== value number `operation` 10
        startTimer holdTimer 500



updateIndicator = call "updateIndicator"
{-    
setLED v = action (\[x] -> "setLED(" ++ x ++ ")") [v']
    where v' = ue . value $ v    
-}          
