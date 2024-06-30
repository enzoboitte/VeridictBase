to="../assets/scripts/SCENE.h"
rm "$to"
printf -v var "%s\n" "#include \"VerCore/IReflectable.hpp\"" \
              "#include \"VerCore/SceneManager.hpp\"" \
              "USCENEDECL()"

echo "$var" > "$to"

search_dir=../assets/scenes/
iterate=0

for entry in "$search_dir"*".json"
do
  out="$(basename $entry .json)"
  if [ "$out" != "*" ]; then
    echo "$entry"
    if [ $iterate == 0 ]; then
        ./rxxd "$entry" "$to" -n "$out" -c
      else
        ./rxxd "$entry" "$to" -n "$out" -c
      fi

      echo USCENE\("$out", "$out"_len\) >> "$to"
      ((iterate++))
  fi
done

cmake .
make
