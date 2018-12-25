delete from m_imu where id_m_imu in
(select * from
	(
	select distinct id_m_imu as id from m_imu 
	   left join m_data on fk_id_m_imu=id_m_imu 
	   group by 1
	   having count(time_raw)=0
	) as t 
)
   