import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.UUID;
import java.util.stream.IntStream;

public class NormBenchmark {

    static int N;

    public static void main(String[] args) {
        List<Float> U = new ArrayList<Float>();
        List<Integer> sizes = new ArrayList<Integer>();
        List<String> results = new ArrayList<>();

        for (int k = 0; k < 10; k++){
            sizes.add(1000);sizes.add(10000);sizes.add(100000);
            sizes.add(1000000);
        }

        // type fonction,n,threads,time
                for (int z = 0 ; z < sizes.size(); z++){
                    U.clear();
                    N = 32 * sizes.get(z);
                    for (int i = 0 ; i < N ; i++){
                        U.add((float)1);
                    }
                // for-loop old school 1M
                Float res = (float) 0.;
                Instant start = Instant.now();
                
                for (int i = 0; i < N; i++) {
                    res += (float)Math.sqrt(Math.abs(U.get(i)));
                }
    
                Instant end = Instant.now();
                Duration timeElapsed = Duration.between(start, end);
                results.add(String.format("JAVA-Traditional for-loop,%s,%s,%s",N,1,timeElapsed.toNanos()/1000.));
    
                // iterator 1M
                Iterator<Float> iterator = U.iterator();
                res = (float) 0.;
                start = Instant.now();
    
                while (iterator.hasNext()) {
                    res += (float)Math.sqrt(Math.abs(iterator.next()));
                }
    
                end = Instant.now();
                timeElapsed = Duration.between(start, end);
                results.add(String.format("JAVA-Traditional iterator-loop,%s,%s,%s",N,1,timeElapsed.toNanos()/1000.));
                // 1M for-loop object
                res = (float) 0.;
                start = Instant.now();
                for (Float item : U) {
                    res += (float)Math.sqrt(Math.abs(item));
                }
                end = Instant.now();
                timeElapsed = Duration.between(start, end);
                results.add(String.format("JAVA-for-loop object,%s,%s,%s",N,1,timeElapsed.toNanos()/1000.));
                // 1M for-each lambda
                res = (float) 0.;
                start = Instant.now();
                U.forEach(val -> Math.sqrt(Math.abs(val)));
                res = (float)U.stream()
                .mapToDouble(a -> a)
                .sum();
                end = Instant.now();
                timeElapsed = Duration.between(start, end);
                results.add(String.format("JAVA-for-each lambda,%s,%s,%s",N,1,timeElapsed.toNanos()/1000.));
                // 1M parallel stream for-each lambda
                res = (float) 0.;
                start = Instant.now();
                U.parallelStream().forEach(val -> Math.sqrt(Math.abs(val)));
                res = (float)U.parallelStream()
                .mapToDouble(a -> a)
                .sum();
                end = Instant.now();
                timeElapsed = Duration.between(start, end);
                results.add(String.format("JAVA-parallel stream,%s,%s,%s",N,1,timeElapsed.toNanos()/1000.));
            
            }
        Collections.sort(results);
        results.forEach(System.out::println);

    }
}

/*

Traditional for-loop 1M: 271 milliseconds - experiment 5
Traditional for-loop 1M: 275 milliseconds - experiment 6
Traditional for-loop 1M: 279 milliseconds - experiment 8
Traditional for-loop 1M: 284 milliseconds - experiment 9
Traditional for-loop 1M: 295 milliseconds - experiment 2
Traditional for-loop 1M: 300 milliseconds - experiment 7
Traditional for-loop 1M: 334 milliseconds - experiment 4
Traditional for-loop 1M: 583 milliseconds - experiment 0
Traditional for-loop 1M: 641 milliseconds - experiment 3
Traditional for-loop 1M: 784 milliseconds - experiment 1
Traditional iterator 1M: 324 milliseconds - experiment 9
Traditional iterator 1M: 325 milliseconds - experiment 2
Traditional iterator 1M: 332 milliseconds - experiment 5
Traditional iterator 1M: 334 milliseconds - experiment 4
Traditional iterator 1M: 336 milliseconds - experiment 3
Traditional iterator 1M: 345 milliseconds - experiment 6
Traditional iterator 1M: 345 milliseconds - experiment 8
Traditional iterator 1M: 349 milliseconds - experiment 1
Traditional iterator 1M: 364 milliseconds - experiment 7
Traditional iterator 1M: 764 milliseconds - experiment 0
for-each lambda 1M: 512 milliseconds - experiment 5
for-each lambda 1M: 513 milliseconds - experiment 4
for-each lambda 1M: 515 milliseconds - experiment 1
for-each lambda 1M: 518 milliseconds - experiment 2
for-each lambda 1M: 520 milliseconds - experiment 3
for-each lambda 1M: 523 milliseconds - experiment 8
for-each lambda 1M: 530 milliseconds - experiment 7
for-each lambda 1M: 539 milliseconds - experiment 0
for-each lambda 1M: 540 milliseconds - experiment 6
for-each lambda 1M: 593 milliseconds - experiment 9
for-loop object 1M: 324 milliseconds - experiment 1
for-loop object 1M: 331 milliseconds - experiment 2
for-loop object 1M: 332 milliseconds - experiment 4
for-loop object 1M: 332 milliseconds - experiment 7
for-loop object 1M: 335 milliseconds - experiment 9
for-loop object 1M: 340 milliseconds - experiment 5
for-loop object 1M: 342 milliseconds - experiment 3
for-loop object 1M: 344 milliseconds - experiment 6
for-loop object 1M: 351 milliseconds - experiment 8
for-loop object 1M: 652 milliseconds - experiment 0
parallel stream for-each lambda 1M: 100 milliseconds - experiment 7
parallel stream for-each lambda 1M: 103 milliseconds - experiment 4
parallel stream for-each lambda 1M: 103 milliseconds - experiment 6
parallel stream for-each lambda 1M: 106 milliseconds - experiment 1
parallel stream for-each lambda 1M: 176 milliseconds - experiment 0
parallel stream for-each lambda 1M: 92 milliseconds - experiment 8
parallel stream for-each lambda 1M: 94 milliseconds - experiment 3
parallel stream for-each lambda 1M: 94 milliseconds - experiment 9
parallel stream for-each lambda 1M: 95 milliseconds - experiment 2
parallel stream for-each lambda 1M: 98 milliseconds - experiment 5

*/

/* arthur 

Traditional for-loop 1M: 101 milliseconds - experiment 4
Traditional for-loop 1M: 102 milliseconds - experiment 5
Traditional for-loop 1M: 104 milliseconds - experiment 2
Traditional for-loop 1M: 106 milliseconds - experiment 6
Traditional for-loop 1M: 106 milliseconds - experiment 7
Traditional for-loop 1M: 106 milliseconds - experiment 9
Traditional for-loop 1M: 108 milliseconds - experiment 3
Traditional for-loop 1M: 306 milliseconds - experiment 1
Traditional for-loop 1M: 417 milliseconds - experiment 0
Traditional for-loop 1M: 99 milliseconds - experiment 8
Traditional iterator 1M: 101 milliseconds - experiment 3
Traditional iterator 1M: 102 milliseconds - experiment 5
Traditional iterator 1M: 103 milliseconds - experiment 4
Traditional iterator 1M: 103 milliseconds - experiment 9
Traditional iterator 1M: 104 milliseconds - experiment 8
Traditional iterator 1M: 105 milliseconds - experiment 2
Traditional iterator 1M: 106 milliseconds - experiment 6
Traditional iterator 1M: 107 milliseconds - experiment 7
Traditional iterator 1M: 323 milliseconds - experiment 0
Traditional iterator 1M: 99 milliseconds - experiment 1
for-each lambda 1M: 428 milliseconds - experiment 4
for-each lambda 1M: 432 milliseconds - experiment 1
for-each lambda 1M: 435 milliseconds - experiment 0
for-each lambda 1M: 435 milliseconds - experiment 3
for-each lambda 1M: 437 milliseconds - experiment 7
for-each lambda 1M: 439 milliseconds - experiment 9
for-each lambda 1M: 440 milliseconds - experiment 6
for-each lambda 1M: 443 milliseconds - experiment 8
for-each lambda 1M: 444 milliseconds - experiment 5
for-each lambda 1M: 449 milliseconds - experiment 2
for-loop object 1M: 172 milliseconds - experiment 4
for-loop object 1M: 177 milliseconds - experiment 3
for-loop object 1M: 180 milliseconds - experiment 5
for-loop object 1M: 181 milliseconds - experiment 7
for-loop object 1M: 181 milliseconds - experiment 9
for-loop object 1M: 182 milliseconds - experiment 6
for-loop object 1M: 185 milliseconds - experiment 1
for-loop object 1M: 185 milliseconds - experiment 8
for-loop object 1M: 233 milliseconds - experiment 0
for-loop object 1M: 254 milliseconds - experiment 2
parallel stream for-each lambda 1M: 138 milliseconds - experiment 0
parallel stream for-each lambda 1M: 82 milliseconds - experiment 5
parallel stream for-each lambda 1M: 82 milliseconds - experiment 9
parallel stream for-each lambda 1M: 83 milliseconds - experiment 2
parallel stream for-each lambda 1M: 83 milliseconds - experiment 4
parallel stream for-each lambda 1M: 83 milliseconds - experiment 7
parallel stream for-each lambda 1M: 83 milliseconds - experiment 8
parallel stream for-each lambda 1M: 84 milliseconds - experiment 1
parallel stream for-each lambda 1M: 84 milliseconds - experiment 3
parallel stream for-each lambda 1M: 90 milliseconds - experiment 6

*/