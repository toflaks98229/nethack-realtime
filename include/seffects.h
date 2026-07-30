/* NetHack 5.0	seffects.h	$NHDT-Date: 1781973087 2026/06/20 16:31:27 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.7 $ */
/* Copyright (c) Michael Allison, 2023                                */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file seffects.h
 * @brief The list of every sound effect the game can ask for, written once.
 *
 * A sound effect needs to exist twice: as a value the game passes around, and as a name a sound library looks up. Writing both lists by hand would let them drift, so this file is
 * the single list and is read twice -- once with a macro that turns each entry into an enumerator, once with a macro that turns it into a name-and-value pair.
 *
 * So this is not a header in the usual sense. It has no include guard and defines nothing on its own; it is a list, and what it produces depends on which macro the includer defined
 * first.
 *
 * The names are descriptive of the sound rather than of the situation -- a squeaking board, a growling canine -- which is what lets one effect serve wherever that sound belongs and
 * lets a sound library supply a file per sound without knowing the game.
 *
 * @note Alphabetical, and worth keeping so: it is long, and a name is looked for by eye more often than by search.
 * @note The macro is undefined at the end, so the file can be read again for the other purpose without interference.
 * @warning Adding an entry adds both the value and the name, which is the point. Removing one silently changes the numbering of everything after it -- harmless within a build, but
 *          not if a value has been written down anywhere outside it.
 */

/**
 * @file seffects.h
 * @brief 게임이 요청할 수 있는 모든 음향 효과의 목록. 한 번만 적힌 것.
 *
 * 음향 효과는 두 번 존재해야 한다. 게임이 주고받는 값으로, 그리고 사운드 라이브러리가 찾아보는 이름으로. 두 목록을 손으로 적으면 서로 어긋날 수 있으므로, 이 파일이 그 하나뿐인 목록이고 두 번 읽힌다. 한 번은 각 항목을 열거자로 바꾸는 매크로와 함께, 한 번은 그것을
 * 이름과 값의 짝으로 바꾸는 매크로와 함께.
 *
 * 그래서 이것은 통상적인 의미의 헤더가 아니다. 포함 보호가 없고 스스로는 아무것도 정의하지 않는다. 그것은 목록이며, 무엇을 만들어 내는지는 포함하는 쪽이 먼저 어느 매크로를 정의했는지에 달려 있다.
 *
 * 이름들은 상황이 아니라 소리를 기술한다. 삐걱이는 널판, 으르렁거리는 개. 그것이 하나의 효과를 그 소리가 어울리는 어디서든 쓸 수 있게 하고, 사운드 라이브러리가 게임을 모른 채로 소리마다 파일을 제공할 수 있게 하는 것이다.
 *
 * @note 알파벳 순서이며 그렇게 유지할 가치가 있다. 목록이 길고, 이름은 검색보다 눈으로 찾는 일이 더 많다.
 * @note 매크로는 끝에서 정의 해제되므로, 이 파일을 다른 목적으로 다시 읽을 때 방해가 없다.
 * @warning 항목을 더하는 것은 값과 이름을 함께 더하는 일이며, 그것이 요점이다. 하나를 없애면 그 뒤의 모든 것의 번호가 조용히 바뀐다. 한 빌드 안에서는 해롭지 않지만, 어떤 값이 그 밖에 적혀 있다면 그렇지 않다.
 */

#if defined(SEFFECTS_ENUM) || defined(SEFFECTS_AUTOMAP)

/**
 * @def seffect
 * @brief One entry of the list, expanded according to what the includer asked for.
 * @param basename the effect's descriptive name
 * @note Two expansions from one entry: an enumerator, or a pair of that enumerator and its name as text. That is what keeps the values and the names in step.
 * @warning Not usable outside this file. It is defined here, used, and undefined again -- the includer supplies only the choice of which expansion.
 */
/**
 * @def seffect
 * @brief 목록의 한 항목. 포함하는 쪽이 요청한 것에 따라 펼쳐진다.
 * @param basename 그 효과의 기술적인 이름
 * @note 하나의 항목에서 두 가지 펼침. 열거자, 또는 그 열거자와 그것의 글로서의 이름의 짝. 그것이 값과 이름을 보조를 맞추게 하는 것이다.
 * @warning 이 파일 밖에서는 쓸 수 없다. 여기서 정의되고 쓰이고 다시 정의 해제된다. 포함하는 쪽은 어느 펼침인지의 선택만을 제공한다.
 */
#if defined(SEFFECTS_ENUM)
#define seffect(basename) se_##basename
#else
#if defined(SEFFECTS_AUTOMAP)
#define seffect(basename) \
    { se_##basename, #basename }
#endif
#endif
    seffect(air_crackles),
    seffect(alarm),
    seffect(angry_drone),
    seffect(angry_snakes),
    seffect(angry_voice),
    seffect(applause),
    seffect(avian_screak),
    seffect(bang_weapon_side),
    seffect(bars_clink),
    seffect(bars_clonk),
    seffect(bars_flapp),
    seffect(bars_whang),
    seffect(bars_whap),
    seffect(bees),
    seffect(blast),
    seffect(board_squeak),
    seffect(board_squeaks_loudly),
    seffect(boing),
    seffect(bolt_of_lightning),
    seffect(bone_rattle),
    seffect(boomerang_klonk),
    seffect(boulder_drop),
    seffect(bovine_bellow),
    seffect(bovine_moo),
    seffect(bubble_rising),
    seffect(bugle_playing_reveille),
    seffect(buzz),
    seffect(canine_bark),
    seffect(canine_growl),
    seffect(canine_howl),
    seffect(canine_whine),
    seffect(canine_yelp),
    seffect(canine_yip),
    seffect(canine_yowl),
    seffect(chain_shatters),
    seffect(chains_rattling_gears_turning),
    seffect(chant),
    seffect(chirp),
    seffect(clanging_sound),
    seffect(clank),
    seffect(clanking_pipe),
    seffect(clash),
    seffect(cockatrice_hiss),
    seffect(cough),
    seffect(courtly_conversation),
    seffect(cracking_sound),
    seffect(crackling),
    seffect(crackling_of_hellfire),
    seffect(crash),
    seffect(crash_door),
    seffect(crash_something_broke),
    seffect(crash_throne_destroyed),
    seffect(crash_through_floor),
    seffect(crashed_ceiling),
    seffect(crashing_boulder),
    seffect(crashing_rock),
    seffect(crashing_sound),
    seffect(croc_bellow),
    seffect(crumbling_sound),
    seffect(crunching_sound),
    seffect(crushing_sound),
    seffect(deafening_roar_atmospheric),
    seffect(destroy_web),
    seffect(distant_thunder),
    seffect(divine_music),
    seffect(door_crash_open),
    seffect(door_open),
    seffect(door_unlock_and_open),
    seffect(drain_noises),
    seffect(dry_throat_rattle),
    seffect(egg_cracking),
    seffect(egg_splatting),
    seffect(elephant_trumpet),
    seffect(equine_neigh),
    seffect(equine_whicker),
    seffect(equine_whinny),
    seffect(explosion),
    seffect(faint_chime),
    seffect(faint_sloshing),
    seffect(faint_splashing),
    seffect(feline_meow),
    seffect(feline_mew),
    seffect(feline_purr),
    seffect(feline_yelp),
    seffect(feline_yip),
    seffect(feline_yowl),
    seffect(furious_bubbling),
    seffect(gear_turn),
    seffect(gears_turning_chains_rattling),
    seffect(glass_crashing),
    seffect(glass_shattering),
    seffect(groan),
    seffect(groans_and_moans),
    seffect(growl),
    seffect(grunt),
    seffect(guards_footsteps),
    seffect(gurgle),
    seffect(gushing_sound),
    seffect(heart_beat),
    seffect(hiss),
    seffect(hollow_sound),
    seffect(horn_being_played),
    seffect(iron_ball_dragging_you),
    seffect(iron_ball_hits_you),
    seffect(item_tumble_downwards),
    seffect(jabberwock_burble),
    seffect(kaablamm_of_mine),
    seffect(kaboom),
    seffect(kaboom_boom_boom),
    seffect(kaboom_door_explodes),
    seffect(kadoom_boulder_falls_in),
    seffect(kerplunk_boulder_gone),
    seffect(kick_door_it_crashes_open),
    seffect(kick_door_it_shatters),
    seffect(klick),
    seffect(klunk),
    seffect(klunk_pipe),
    seffect(laughter),
    seffect(lid_slams_open_falls_shut),
    seffect(loud_click),
    seffect(loud_crash),
    seffect(loud_pop),
    seffect(loud_splash),
    seffect(low_buzzing),
    seffect(low_hum),
    seffect(maniacal_laughter),
    seffect(masticating_sound),
    seffect(mon_chugging_potion),
    seffect(monster_behind_boulder),
    seffect(mutter_imprecations),
    seffect(mutter_incantation),
    seffect(orc_grunt),
    seffect(paranoid_confirmation),
    seffect(potion_crash_and_break),
    seffect(ring_in_drain),
    seffect(ripping_sound),
    seffect(snarl),
    seffect(roar),
    seffect(rumbling),
    seffect(rumbling_of_earth),
    seffect(rushing_wind_noise),
    seffect(rustling_paper),
    seffect(sad_wailing),
    seffect(sceptor_pounding),
    seffect(scratching),
    seffect(scream),
    seffect(screech),
    seffect(sewer_song),
    seffect(sharp_crack),
    seffect(shriek),
    seffect(shrill_whistle),
    seffect(sinister_laughter),
    seffect(sizzling),
    seffect(slurping_sound),
    seffect(smashing_and_crushing),
    seffect(snake_rattle),
    seffect(snakes_hissing),
    seffect(soft_click),
    seffect(soft_crackling),
    seffect(someone_bowling),
    seffect(someone_searching),
    seffect(someone_summoning),
    seffect(someone_yells),
    seffect(splash),
    seffect(splat_egg),
    seffect(splat_from_engulf),
    seffect(squawk),
    seffect(squeak),
    seffect(squeak_A),
    seffect(squeak_B),
    seffect(squeak_B_flat),
    seffect(squeak_C),
    seffect(squeak_D),
    seffect(squeak_D_flat),
    seffect(squeak_E),
    seffect(squeak_E_flat),
    seffect(squeak_F),
    seffect(squeak_F_sharp),
    seffect(squeak_G),
    seffect(squeak_G_sharp),
    seffect(squeal),
    seffect(squelch),
    seffect(stone_breaking),
    seffect(stone_crumbling),
    seffect(swoosh),
    seffect(sword_blade_rings),
    seffect(thud),
    seffect(thump),
    seffect(thunderclap),
    seffect(tumbler_click),
    seffect(typing_noise),
    seffect(wail),
    seffect(wailing_of_the_banshee),
    seffect(wall_of_force),
    seffect(yelp),
    seffect(zap),
    seffect(zap_then_explosion),
#undef seffect
#endif   /* SEFFECTS_ENUM || SEFFECTS_AUTOMAP */

/* seffects.h */

